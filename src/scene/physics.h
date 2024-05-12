#ifndef PHYSICS_H
#define PHYSICS_H

#include <bullet/btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../scene/mesh/meshModel.h"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_vulkan.h"
#if defined(_WIN32)
#include "../../imgui/imgui_impl_win32.h"
#endif
#include "../../imgui/ImGuizmo.h"
#include "../../imgui/imgui_internal.h"

class Physics
{
    friend class Viewport;

    btDefaultCollisionConfiguration* collisionConfiguration_;
    btCollisionDispatcher* dispatcher_;
    btBroadphaseInterface* overlappingPairCache_;
    btSequentialImpulseConstraintSolver* solver_;
    btDiscreteDynamicsWorld* dynamicsWorld_;

    btAlignedObjectArray<btCollisionShape*> collisionShapes_;

    bool firstStep;

public:
    void InitPhysics();
    void AddRigidBodies(std::vector<MeshModel>& meshes);
    void Simulate(std::vector<MeshModel>& meshes);
    void Stop(std::vector<MeshModel>& meshes);
    ~Physics();
};

#endif