#ifndef PHYSICS_H
#define PHYSICS_H

#include <bullet/btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../scene/mesh/meshModel.h"

class Physics
{
    btDefaultCollisionConfiguration* collisionConfiguration_;
    btCollisionDispatcher* dispatcher_;
    btBroadphaseInterface* overlappingPairCache_;
    btSequentialImpulseConstraintSolver* solver_;
    btDiscreteDynamicsWorld* dynamicsWorld_;

    btAlignedObjectArray<btCollisionShape*> collisionShapes_;

    std::vector<std::vector<MeshInstanceUBO>> meshInstanceCopies_;

public:
    void InitPhysics();
    void RegisterMeshes(std::vector<MeshModel>& meshes);
    void Simulate(Mesh& mesh);
    void Stop(std::vector<MeshModel>& meshes);
};

#endif