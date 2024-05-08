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

void Physics::AddRigidBody(MeshInstanceUBO& ubo, const MeshInstancePhysicsInfo& pInfo)
{
    ubo.pInfo = std::make_unique<MeshInstancePhysicsInfo>(pInfo);

    btCollisionShape* shape;
    switch (ubo.pInfo->rigidBodyShape) {
    case (eRigidBodyShape::BOX):
        shape = new btBoxShape({ 0.5f, 0.5f, 0.5f });
        break;
    case (eRigidBodyShape::SPHERE):
        shape = new btSphereShape(1.0f);
        break;
    case (eRigidBodyShape::CAPSULE):
        shape = new btCapsuleShape({ 1.0f, 1.0f });
        break;
    case (eRigidBodyShape::CYLINDER):
        shape = new btCylinderShape({ 0.5f, 0.5f, 0.5f });
        break;
    case (eRigidBodyShape::CONE):
        shape = new btConeShape({ 1.0f, 1.0f });
        break;
    default:
        return;
    }

    shape->setLocalScaling({ ubo.pInfo->scale[0], ubo.pInfo->scale[1], ubo.pInfo->scale[2] });
    collisionShapes_.push_back(shape);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin({ ubo.model[3].x, ubo.model[3].y, ubo.model[3].z });

    btScalar mass = ubo.pInfo->rigidBodyType == eRigidBodyType::DYNAMIC ? 1.0f : 0.0f;
    btVector3 localInertia(0, 0, 0);
    shape->calculateLocalInertia(mass, localInertia);

    auto* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
    auto* body = new btRigidBody(cInfo);
    btTransform t;
    t.setIdentity();
    t.setFromOpenGLMatrix(glm::value_ptr(ubo.pInfo->matrix));
    body->setWorldTransform(t);
    ubo.pInfo->rigidBodyPtr = body;

    dynamicsWorld_->addRigidBody(body);

    ubo.pInfo->initialModel = ubo.model;

    DebugDraw();
}

void Physics::DeleteRigidBody(MeshInstanceUBO& ubo)
{
    if (!ubo.pInfo)
        return;
    dynamicsWorld_->removeRigidBody(ubo.pInfo->rigidBodyPtr);
    delete ubo.pInfo->rigidBodyPtr;
    ubo.pInfo.reset();

    DebugDraw();
}

void Physics::UpdateRigidBody(const MeshInstanceUBO& ubo)
{
    ubo.pInfo->initialModel = ubo.model;

    auto body = ubo.pInfo->rigidBodyPtr;
    body->getCollisionShape()->setLocalScaling({ ubo.pInfo->scale[0], ubo.pInfo->scale[1], ubo.pInfo->scale[2] });
    btTransform t;
    t.setIdentity();
    t.setFromOpenGLMatrix(glm::value_ptr(ubo.pInfo->matrix));
    body->setWorldTransform(t);

    DebugDraw();
}

void Physics::Simulate(std::vector<MeshModel>& meshes)
{
    dynamicsWorld_->stepSimulation(1.0f / 60.0f);

    for (auto& mesh : meshes) {
        for (auto& instanceUBO : mesh.meshInstanceUBOs_) {
            if (!instanceUBO.pInfo)
                continue;
            auto body = instanceUBO.pInfo->rigidBodyPtr;
            auto t = body->getWorldTransform();
            auto& s = body->getCollisionShape()->getLocalScaling();
            btScalar m[16];
            t.getOpenGLMatrix(m);
            instanceUBO.model = glm::scale(glm::make_mat4(m), glm::vec3(s.x(), s.y(), s.z()) / instanceUBO.pInfo->size);

            int f = body->getCollisionFlags();
            body->setCollisionFlags(f | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

            instanceUBO.invTranspose = instanceUBO.model;
            instanceUBO.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            instanceUBO.invTranspose = glm::transpose(glm::inverse(instanceUBO.invTranspose));
        }
    }

    DebugDraw();
}

void Physics::Stop(std::vector<MeshModel>& meshes)
{
    for (auto& mesh : meshes) {
        for (auto& instanceUBO : mesh.meshInstanceUBOs_) {
            if (!instanceUBO.pInfo)
                continue;
            instanceUBO.model = instanceUBO.pInfo->initialModel;

            auto body = instanceUBO.pInfo->rigidBodyPtr;
            body->clearForces();
            body->setLinearVelocity({ 0, 0, 0 });
            body->setAngularVelocity({ 0, 0, 0 });
            btTransform t;
            t.setIdentity();
            t.setFromOpenGLMatrix(glm::value_ptr(instanceUBO.pInfo->matrix));
            body->setWorldTransform(t);

            int f = body->getCollisionFlags();
            body->setCollisionFlags(f ^ btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

            instanceUBO.invTranspose = instanceUBO.model;
            instanceUBO.invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            instanceUBO.invTranspose = glm::transpose(glm::inverse(instanceUBO.invTranspose));
        }
    }

    DebugDraw();
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
