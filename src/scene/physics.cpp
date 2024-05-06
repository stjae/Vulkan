#include "physics.h"

void Physics::InitPhysics()
{
    collisionConfiguration_ = new btDefaultCollisionConfiguration();
    dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
    overlappingPairCache_ = new btDbvtBroadphase();
    solver_ = new btSequentialImpulseConstraintSolver;
    dynamicsWorld_ = new btDiscreteDynamicsWorld(dispatcher_, overlappingPairCache_, solver_, collisionConfiguration_);
    dynamicsWorld_->setGravity(btVector3(0, -10, 0));
    dynamicsWorld_->setDebugDrawer((btIDebugDraw*)&debugDrawer_);
    dynamicsWorld_->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    vkn::Command::CreateCommandPool(debugDrawer_.m_commandPool);
    vkn::Command::AllocateCommandBuffer(debugDrawer_.m_commandPool, debugDrawer_.m_commandBuffer);
}

void Physics::AddRigidBody(const MeshInstanceUBO& ubo, MeshInstancePhysicsInfo& pInfo, float* matrix, float* scale)
{
    btCollisionShape* shape;
    switch (pInfo.shape) {
    case (ePhysicsShape::BOX):
        shape = new btBoxShape({ 1.0f, 1.0f, 1.0f });
        break;
    case (ePhysicsShape::SPHERE):
        shape = new btSphereShape(1.0f);
        break;
    case (ePhysicsShape::CAPSULE):
        shape = new btCapsuleShape({ 1.0f, 1.0f });
        break;
    case (ePhysicsShape::CYLINDER):
        shape = new btCylinderShape({ 1.0f, 1.0f, 1.0f });
        break;
    case (ePhysicsShape::CONE):
        shape = new btConeShape({ 1.0f, 1.0f });
        break;
    default:
        return;
    }

    shape->setLocalScaling({ scale[0], scale[1], scale[2] });
    collisionShapes_.push_back(shape);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin({ ubo.model[3].x, ubo.model[3].y, ubo.model[3].z });

    btScalar mass = pInfo.type == ePhysicsType::DYNAMIC ? 1.0f : 0.0f;
    btVector3 localInertia(0, 0, 0);
    shape->calculateLocalInertia(mass, localInertia);

    auto* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
    auto* body = new btRigidBody(cInfo);
    btTransform t;
    t.setIdentity();
    t.setFromOpenGLMatrix(matrix);
    body->setWorldTransform(t);
    pInfo.rigidBodyPtr = body;

    dynamicsWorld_->addRigidBody(body);
    pInfo.haveRigidBody = true;

    pInfo.initialModel = ubo.model;

    DebugDraw();
}

void Physics::DeleteRigidBody(MeshInstancePhysicsInfo& pInfo)
{
    dynamicsWorld_->removeRigidBody(pInfo.rigidBodyPtr);
    delete pInfo.rigidBodyPtr;
    pInfo.haveRigidBody = false;

    DebugDraw();
}

void Physics::UpdateRigidBody(btRigidBody* rigidBody, float* matrix, float* scale)
{
    rigidBody->getCollisionShape()->setLocalScaling({ scale[0], scale[1], scale[2] });
    btTransform t;
    t.setIdentity();
    t.setFromOpenGLMatrix(matrix);
    rigidBody->setWorldTransform(t);

    DebugDraw();
}

void Physics::Simulate(std::vector<MeshModel>& meshes)
{
    dynamicsWorld_->stepSimulation(1.0f / 60.0f);

    for (auto& mesh : meshes) {
        for (int i = 0; i < mesh.GetInstanceCount(); i++) {
            if (!mesh.meshInstancePhysicsInfos_[i].haveRigidBody)
                continue;
            auto t = mesh.meshInstancePhysicsInfos_[i].rigidBodyPtr->getWorldTransform();
            auto& s = mesh.meshInstancePhysicsInfos_[i].rigidBodyPtr->getCollisionShape()->getLocalScaling();
            btScalar m[16];
            t.getOpenGLMatrix(m);
            // mesh.meshInstanceUBOs_[i].model = glm::scale(glm::make_mat4(m), glm::vec3(s.x(), s.y(), s.z()));
            mesh.meshInstanceUBOs_[i].model = glm::make_mat4(m);
        }
    }
}

void Physics::Stop(std::vector<MeshModel>& meshes)
{
    for (auto& mesh : meshes) {
        for (int i = 0; i < mesh.GetInstanceCount(); i++) {
            if (!mesh.meshInstancePhysicsInfos_[i].haveRigidBody)
                continue;
            mesh.meshInstanceUBOs_[i].model = mesh.meshInstancePhysicsInfos_[i].initialModel;

            auto body = mesh.meshInstancePhysicsInfos_[i].rigidBodyPtr;
            body->clearForces();
            body->setLinearVelocity({ 0, 0, 0 });
            body->setAngularVelocity({ 0, 0, 0 });
            btTransform t;
            t.setIdentity();
            t.setFromOpenGLMatrix(glm::value_ptr(mesh.meshInstanceUBOs_[i].model));
            body->setWorldTransform(t);
        }
    }
}

Physics::~Physics()
{
    for (int i = 0; i < dynamicsWorld_->getNumCollisionObjects(); i++) {
        auto obj = dynamicsWorld_->getCollisionObjectArray()[i];
        auto body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        dynamicsWorld_->removeCollisionObject(obj);
        delete obj;
    }

    for (int i = 0; i < collisionShapes_.size(); i++) {
        auto shape = collisionShapes_[i];
        collisionShapes_[i] = 0;
        delete shape;
    }

    delete dynamicsWorld_;
    delete solver_;
    delete overlappingPairCache_;
    delete dispatcher_;
}
