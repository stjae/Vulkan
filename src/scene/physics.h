#ifndef PHYSICS_H
#define PHYSICS_H

#include <bullet/btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include "../scene/mesh.h"
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

    inline static btDefaultCollisionConfiguration* s_collisionConfiguration;
    inline static btCollisionDispatcher* s_dispatcher;
    inline static btBroadphaseInterface* s_overlappingPairCache;
    inline static btSequentialImpulseConstraintSolver* s_solver;
    inline static btDiscreteDynamicsWorld* s_dynamicsWorld;
    inline static btAlignedObjectArray<btCollisionShape*> s_collisionShapes;

    inline static bool s_isFirstStep = true;

public:
    static void InitPhysics();
    static void AddRigidBody(Mesh& mesh, MeshInstance& meshInstance);
    static void DeleteRigidBody(MeshInstance& instance);
    static void UpdateRigidBodies(std::vector<std::shared_ptr<Mesh>>& meshes);
    static void Simulate(std::vector<std::shared_ptr<Mesh>>& meshes);
    ~Physics();
};

#endif