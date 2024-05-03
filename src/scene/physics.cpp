#include "physics.h"

void Physics::InitPhysics()
{
    collisionConfiguration_ = new btDefaultCollisionConfiguration();
    dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
    overlappingPairCache_ = new btDbvtBroadphase();
    solver_ = new btSequentialImpulseConstraintSolver;
    dynamicsWorld_ = new btDiscreteDynamicsWorld(dispatcher_, overlappingPairCache_, solver_, collisionConfiguration_);
    dynamicsWorld_->setGravity(btVector3(0, -10, 0));
}

void Physics::RegisterMeshes(std::vector<MeshModel>& meshes)
{
    btBoxShape* box = new btBoxShape({ 1.0f, 1.0f, 1.0f });
    collisionShapes_.push_back(box);

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(0, 0, 0));

    btScalar mass(1.f);
    btVector3 localInertia(0, 0, 0);
    box->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, box, localInertia);
    btRigidBody* body = new btRigidBody(cInfo);

    dynamicsWorld_->addRigidBody(body);

    // TODO: store the initial model matrix of mesh instances
    for (auto& mesh : meshes) {
        if (mesh.GetInstanceCount() > 0) {
            meshInstanceCopies_.emplace_back();
            for (auto& instanceUBO : mesh.meshInstances_) {
                meshInstanceCopies_.back().emplace_back(instanceUBO);
            }
        }
    }
}

void Physics::Simulate(Mesh& mesh)
{
    dynamicsWorld_->stepSimulation(1.0f / 60.0f);

    btCollisionObject* obj = dynamicsWorld_->getCollisionObjectArray()[0];
    btRigidBody* body = btRigidBody::upcast(obj);
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    std::cout << trans.getOrigin().getX() << ' '
              << trans.getOrigin().getY() << ' '
              << trans.getOrigin().getZ() << '\n';

    mesh.meshInstances_[0].model = glm::translate(glm::mat4(1.0f), glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
}

void Physics::Stop(std::vector<MeshModel>& meshes)
{
    // TODO: restore model matrices
    for (int i = 0; i < meshes.size(); i++) {
        auto& uboCopy = meshInstanceCopies_[i];
        for (int j = 0; j < meshes[i].GetInstanceCount(); j++) {
            meshes[i].meshInstances_[j] = uboCopy[j];
            std::cout << glm::to_string(meshes[i].meshInstances_[j].model) << '\n';
        }
    }

    btCollisionObject* obj = dynamicsWorld_->getCollisionObjectArray()[0];
    btRigidBody* body = btRigidBody::upcast(obj);
    body->clearForces();
    body->setLinearVelocity({ 0, 0, 0 });
    body->setAngularVelocity({ 0, 0, 0 });
    btTransform t;
    t.setIdentity();
    body->setWorldTransform(t);
}