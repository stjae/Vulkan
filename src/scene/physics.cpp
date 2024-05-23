#include "physics.h"

void Physics::InitPhysics()
{
    s_collisionConfiguration = new btDefaultCollisionConfiguration();
    s_dispatcher = new btCollisionDispatcher(s_collisionConfiguration);
    s_overlappingPairCache = new btDbvtBroadphase();
    s_solver = new btSequentialImpulseConstraintSolver;
    s_dynamicsWorld = new btDiscreteDynamicsWorld(s_dispatcher, s_overlappingPairCache, s_solver, s_collisionConfiguration);
    s_dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void Physics::AddRigidBody(Mesh& mesh, MeshInstance& instance)
{
    switch (instance.physicsInfo->colliderShape) {
    case (eColliderShape::BOX):
        instance.physicsInfo->collisionShapePtr = new btBoxShape({ 0.5f, 0.5f, 0.5f });
        break;
    case (eColliderShape::SPHERE):
        instance.physicsInfo->collisionShapePtr = new btSphereShape(1.0f);
        break;
    case (eColliderShape::CAPSULE):
        instance.physicsInfo->collisionShapePtr = new btCapsuleShape({ 1.0f, 1.0f });
        break;
    case (eColliderShape::CYLINDER):
        instance.physicsInfo->collisionShapePtr = new btCylinderShape({ 0.5f, 0.5f, 0.5f });
        break;
    case (eColliderShape::CONE):
        instance.physicsInfo->collisionShapePtr = new btConeShape({ 1.0f, 1.0f });
        break;
    case (eColliderShape::MESH):
        instance.physicsInfo->collisionShapePtr = new btBvhTriangleMeshShape(mesh.GetBulletVertexArray(), true);
        break;
    default:
        return;
    }

    float* instanceM = glm::value_ptr(instance.UBO.model);
    float instanceT[3];
    float instanceR[3];
    float instanceS[3];
    ImGuizmo::DecomposeMatrixToComponents(instanceM, instanceT, instanceR, instanceS);

    instance.physicsInfo->collisionShapePtr->setLocalScaling({ instanceS[0] * instance.physicsInfo->scale.x, instanceS[1] * instance.physicsInfo->scale.y, instanceS[2] * instance.physicsInfo->scale.z });
    s_collisionShapes.push_back(instance.physicsInfo->collisionShapePtr);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin({ instanceT[0], instanceT[1], instanceT[2] });

    btScalar mass = instance.physicsInfo->rigidBodyType == eRigidBodyType::DYNAMIC ? 1.0f : 0.0f;
    btVector3 localInertia(0, 0, 0);
    instance.physicsInfo->collisionShapePtr->calculateLocalInertia(mass, localInertia);

    auto* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, instance.physicsInfo->collisionShapePtr, localInertia);
    auto* body = new btRigidBody(cInfo);
    body->setWorldTransform(startTransform);
    s_dynamicsWorld->addRigidBody(body);

    instance.physicsInfo->initialModel = instance.UBO.model;
    instance.physicsInfo->rigidBodyPtr = body;
}

void Physics::UpdateRigidBodies(std::vector<std::shared_ptr<Mesh>>& meshes)
{
    for (auto& mesh : meshes) {
        for (auto& instance : mesh->GetInstances()) {
            if (!instance->physicsInfo)
                continue;

            float* instanceM = glm::value_ptr(instance->UBO.model);
            float instanceT[3];
            float instanceR[3];
            float instanceS[3];
            ImGuizmo::DecomposeMatrixToComponents(instanceM, instanceT, instanceR, instanceS);

            instance->physicsInfo->rigidBodyPtr->getCollisionShape()->setLocalScaling({ instanceS[0] * instance->physicsInfo->scale.x, instanceS[1] * instance->physicsInfo->scale.y, instanceS[2] * instance->physicsInfo->scale.z });

            btTransform startTransform;
            startTransform.setIdentity();
            startTransform.setOrigin({ instanceT[0], instanceT[1], instanceT[2] });

            instance->physicsInfo->rigidBodyPtr->setWorldTransform(startTransform);
            instance->physicsInfo->initialModel = instance->UBO.model;
        }
    }
}

void Physics::Simulate(std::vector<std::shared_ptr<Mesh>>& meshes)
{
    if (s_isFirstStep) {
        UpdateRigidBodies(meshes);
        s_isFirstStep = false;
    }

    s_dynamicsWorld->stepSimulation(1.0f / 60.0f);

    for (auto& mesh : meshes) {
        for (auto& instance : mesh->GetInstances()) {
            if (!instance->physicsInfo)
                continue;
            auto body = instance->physicsInfo->rigidBodyPtr;
            auto t = body->getWorldTransform();
            auto& s = body->getCollisionShape()->getLocalScaling();
            btScalar m[16];
            t.getOpenGLMatrix(m);
            instance->UBO.model = glm::scale(glm::make_mat4(m), glm::vec3(s.x(), s.y(), s.z()) / instance->physicsInfo->scale);
        }
    }
}

void Physics::Stop(std::vector<std::shared_ptr<Mesh>>& meshes)
{
    s_isFirstStep = true;

    for (auto& mesh : meshes) {
        for (auto& instance : mesh->GetInstances()) {
            if (!instance->physicsInfo)
                continue;
            instance->UBO.model = instance->physicsInfo->initialModel;
            instance->physicsInfo->rigidBodyPtr->clearForces();
            btVector3 zeroVector(0, 0, 0);
            instance->physicsInfo->rigidBodyPtr->setLinearVelocity(zeroVector);
            instance->physicsInfo->rigidBodyPtr->setAngularVelocity(zeroVector);
            instance->physicsInfo->rigidBodyPtr->setActivationState(DISABLE_DEACTIVATION);
        }
    }
}

Physics::~Physics()
{
    for (int i = 0; i < s_dynamicsWorld->getNumCollisionObjects(); i++) {
        auto obj = s_dynamicsWorld->getCollisionObjectArray()[i];
        auto body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        s_dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    for (int i = 0; i < s_collisionShapes.size(); i++) {
        auto shape = s_collisionShapes[i];
        s_collisionShapes[i] = 0;
        delete shape;
    }

    delete s_dynamicsWorld;
    delete s_solver;
    delete s_overlappingPairCache;
    delete s_dispatcher;
}

// TODO: move to destructor?
void Physics::DeleteRigidBody(MeshInstance& instance)
{
    if (instance.physicsInfo->rigidBodyPtr->getMotionState())
        delete instance.physicsInfo->rigidBodyPtr->getMotionState();
    s_dynamicsWorld->removeCollisionObject(instance.physicsInfo->rigidBodyPtr);
    delete instance.physicsInfo->rigidBodyPtr;
    s_collisionShapes.remove(instance.physicsInfo->collisionShapePtr);
    delete instance.physicsInfo->collisionShapePtr;
}
