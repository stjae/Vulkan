#include "physics.h"

void Physics::InitPhysics()
{
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
    m_dynamicsWorld->setGravity(btVector3(0, -10, 0));
    m_isFirstStep = true;
}

void Physics::AddRigidBodies(std::vector<MeshModel>& meshes)
{
    for (auto& mesh : meshes) {
        if (!mesh.m_physicsInfo)
            continue;
        for (auto& instance : mesh.m_meshInstanceUBOs) {
            instance.physicsInfo = std::make_unique<MeshInstancePhysicsInfo>();
            btCollisionShape* shape;

            switch (mesh.m_physicsInfo->colliderShape) {
            case (eColliderShape::BOX):
                shape = new btBoxShape({ 0.5f, 0.5f, 0.5f });
                break;
            case (eColliderShape::SPHERE):
                shape = new btSphereShape(1.0f);
                break;
            case (eColliderShape::CAPSULE):
                shape = new btCapsuleShape({ 1.0f, 1.0f });
                break;
            case (eColliderShape::CYLINDER):
                shape = new btCylinderShape({ 0.5f, 0.5f, 0.5f });
                break;
            case (eColliderShape::CONE):
                shape = new btConeShape({ 1.0f, 1.0f });
                break;
            case (eColliderShape::MESH):
                shape = new btBvhTriangleMeshShape(&mesh.m_bulletVertexArray, true);
                break;
            default:
                return;
            }

            float* instanceM = glm::value_ptr(instance.model);
            float instanceT[3];
            float instanceR[3];
            float instanceS[3];
            float colliderM[16];
            ImGuizmo::DecomposeMatrixToComponents(instanceM, instanceT, instanceR, instanceS);
            float unitScale[3] = { 1.0f, 1.0f, 1.0f };
            ImGuizmo::RecomposeMatrixFromComponents(instanceT, instanceR, unitScale, colliderM);

            shape->setLocalScaling({ instanceS[0], instanceS[1], instanceS[2] });
            m_collisionShapes.push_back(shape);

            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin({ instanceT[0], instanceT[1], instanceT[2] });

            btScalar mass = mesh.m_physicsInfo->rigidBodyType == eRigidBodyType::DYNAMIC ? 1.0f : 0.0f;
            btVector3 localInertia(0, 0, 0);
            shape->calculateLocalInertia(mass, localInertia);

            auto* myMotionState = new btDefaultMotionState(startTransform);
            btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
            auto* body = new btRigidBody(cInfo);
            btTransform t;
            t.setIdentity();
            t.setFromOpenGLMatrix(colliderM);
            body->setWorldTransform(t);

            m_dynamicsWorld->addRigidBody(body);

            instance.physicsInfo->initialModel = instance.model;
            instance.physicsInfo->rigidBodyPtr = body;
        }
    }
}

void Physics::Simulate(std::vector<MeshModel>& meshes)
{
    if (m_isFirstStep) {
        AddRigidBodies(meshes);
        m_isFirstStep = false;
    }

    m_dynamicsWorld->stepSimulation(1.0f / 60.0f);

    for (auto& mesh : meshes) {
        if (!mesh.m_physicsInfo)
            continue;
        for (auto& instance : mesh.m_meshInstanceUBOs) {
            auto body = instance.physicsInfo->rigidBodyPtr;
            auto t = body->getWorldTransform();
            auto& s = body->getCollisionShape()->getLocalScaling();
            btScalar m[16];
            t.getOpenGLMatrix(m);
            instance.model = glm::scale(glm::make_mat4(m), glm::vec3(s.x(), s.y(), s.z()) / instance.physicsInfo->size);

            int f = body->getCollisionFlags();
            body->setCollisionFlags(f | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

            instance.invTranspose = instance.model;
            instance.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            instance.invTranspose = glm::transpose(glm::inverse(instance.invTranspose));
        }
    }
}

void Physics::Stop(std::vector<MeshModel>& meshes)
{
    m_isFirstStep = true;

    for (auto& mesh : meshes) {
        if (!mesh.m_physicsInfo)
            continue;
        for (auto& instance : mesh.m_meshInstanceUBOs) {
            instance.model = instance.physicsInfo->initialModel;

            instance.invTranspose = instance.model;
            instance.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            instance.invTranspose = glm::transpose(glm::inverse(instance.invTranspose));
        }
    }

    // cleanup all rigidbody
    std::vector<btCollisionObject*> objs;
    for (int i = 0; i < m_dynamicsWorld->getNumCollisionObjects(); i++) {
        objs.push_back(m_dynamicsWorld->getCollisionObjectArray()[i]);
    }
    for (auto obj : objs) {
        auto body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        m_dynamicsWorld->removeCollisionObject(body);
        delete body;
    }
    for (int i = 0; i < m_collisionShapes.size(); i++) {
        auto shape = m_collisionShapes[i];
        m_collisionShapes[i] = 0;
        delete shape;
    }
}

Physics::~Physics()
{
    for (int i = 0; i < m_dynamicsWorld->getNumCollisionObjects(); i++) {
        auto obj = m_dynamicsWorld->getCollisionObjectArray()[i];
        auto body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        m_dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    for (int i = 0; i < m_collisionShapes.size(); i++) {
        auto shape = m_collisionShapes[i];
        m_collisionShapes[i] = 0;
        delete shape;
    }

    delete m_dynamicsWorld;
    delete m_solver;
    delete m_overlappingPairCache;
    delete m_dispatcher;
}
