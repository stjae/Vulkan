#ifndef PHYSICS_H
#define PHYSICS_H

#include <bullet/btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../scene/mesh/mesh.h"

class Physics
{
        btDefaultCollisionConfiguration* collisionConfiguration_;
        btCollisionDispatcher* dispatcher_;
        btBroadphaseInterface* overlappingPairCache_;
        btSequentialImpulseConstraintSolver* solver_;
        btDiscreteDynamicsWorld* dynamicsWorld_;

        btAlignedObjectArray<btCollisionShape*> collisionShapes_;

public:
    void InitPhysics();
    void RegisterBox();
    void Simulate(Mesh& mesh);
};

#endif