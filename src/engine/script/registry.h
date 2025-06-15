// Definition of C++ functions called from C# scripts
// C#スクリプトから呼び出されるC++関数の定義

#ifndef REGISTRY_H
#define REGISTRY_H

#include <mono/jit/jit.h>
#include <iostream>
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "script.h"
#include "../../scene/scene.h"
#include "../../keycode.h"

inline btVector3 ToBtVector3(const float* v)
{
    return btVector3(v[0], v[1], v[2]);
}

// Internal calls exposed to Mono (C#)
class Registry
{
    static void GetTranslation(uint64_t meshInstanceID, glm::vec3* outTranslation);
    static void SetTranslation(uint64_t meshInstanceID, glm::vec3* inTranslation);
    static void GetRotation(uint64_t meshInstanceID, glm::vec3* outRotation);
    static void SetRotation(uint64_t meshInstanceID, glm::vec3* inRotation);
    static void GetForward(uint64_t meshInstanceID, glm::vec3* outForward);
    static void GetRight(uint64_t meshInstanceID, glm::vec3* outRight);
    static void GetMatrix(uint64_t meshInstanceID, glm::mat4* outMatrix);
    static void GetCameraTranslation(uint64_t meshInstanceID, glm::vec3* outTranslation);
    static void SetCameraTranslation(uint64_t meshInstanceID, glm::vec3* inTranslation);
    static void GetCameraRotation(uint64_t meshInstanceID, glm::vec3* outRotation);
    static void SetCameraRotation(uint64_t meshInstanceID, glm::vec3* inRotation);
    static void GetCameraDirection(uint64_t meshInstanceID, glm::vec3* outDirection);
    static void SetCameraDirection(uint64_t meshInstanceID, glm::vec3* inDirection);
    static bool IsCameraControllable(uint64_t meshInstanceID);
    static bool IsKeyDown(Keycode keycode);
    static bool IsMouseButtonDown(MouseButton mouseButton);
    static float GetMouseX();
    static float GetMouseY();
    static void SetGravity(uint64_t meshInstanceID, const float* inAcceleration);
    static void SetLinearFactor(uint64_t meshInstanceID, const float* inFactor);
    static void SetAngularFactor(uint64_t meshInstanceID, const float* inFactor);
    static void ApplyImpulse(uint64_t meshInstanceID, const float* inImpulse);
    static void GetVelocity(uint64_t meshInstanceID, const float* outVelocity);
    static void SetVelocity(uint64_t meshInstanceID, const float* inVelocity);
    static void SetAngularVelocity(uint64_t meshInstanceID, const float* inVelocity);
    static void SetRigidBodyTransform(uint64_t meshInstanceID, glm::mat4* matrix);
    static void GetRayHitPosition(const float* rayFrom, const float* rayTo, float* hitPosition);
    static void DuplicateMeshInstance(const vk::CommandBuffer& commandBuffer, uint64_t meshInstanceID);

public:
    static void RegisterFunctions();
};

#endif
