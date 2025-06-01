#ifndef PHYSICS_H
#define PHYSICS_H

#include "bullet/btBulletCollisionCommon.h"
#include "bullet/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#include "../scene/mesh.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

class Physics
{
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
    static void UpdateRigidBodies(std::vector<std::unique_ptr<Mesh>>& meshes);
    static void Simulate(std::vector<std::unique_ptr<Mesh>>& meshes);
    static btDiscreteDynamicsWorld* GetDynamicsWorld() { return s_dynamicsWorld; }
    ~Physics();
};

#endif