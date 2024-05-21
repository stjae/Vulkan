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

    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btBroadphaseInterface* m_overlappingPairCache;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;

    btAlignedObjectArray<btCollisionShape*> m_collisionShapes;

    bool m_isFirstStep;

public:
    void InitPhysics();
    void AddRigidBodies(std::vector<Mesh>& meshes);
    void Simulate(std::vector<Mesh>& meshes);
    void Stop(std::vector<Mesh>& meshes);
    ~Physics();
};

#endif