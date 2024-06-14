#include "registry.h"
#include "script.h"
#include "../../scene/scene.h"
#include "../../keycode.h"

static void GetTranslation(uint64_t meshInstanceID, glm::vec3* outTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outTranslation = instance.translation;
}

static void SetTranslation(uint64_t meshInstanceID, glm::vec3* inTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    instance.translation = *inTranslation;
    instance.UpdateMatrix();
    Script::s_scene->GetMeshes()[instance.UBO.meshColorID]->UpdateUBO(instance);
}

static void GetRotation(uint64_t meshInstanceID, glm::vec3* outRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outRotation = instance.rotation;
}

static void SetRotation(uint64_t meshInstanceID, glm::vec3* inRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    instance.rotation = *inRotation;
    instance.UpdateMatrix();
    Script::s_scene->GetMeshes()[instance.UBO.meshColorID]->UpdateUBO(instance);
}

static void GetForward(uint64_t meshInstanceID, glm::vec3* outForward)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outForward = glm::normalize(instance.UBO.model * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
}

static void GetRight(uint64_t meshInstanceID, glm::vec3* outRight)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    glm::vec3 forward = glm::normalize(instance.UBO.model * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    *outRight = glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

static void GetMatrix(uint64_t meshInstanceID, glm::mat4* outMatrix)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outMatrix = instance.UBO.model;
}

static void GetCameraTranslation(uint64_t meshInstanceID, glm::vec3* outTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera.lock()) {
        *outTranslation = instance.camera.lock()->GetTranslation();
    }
}

static void SetCameraTranslation(uint64_t meshInstanceID, glm::vec3* inTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera.lock()) {
        instance.camera.lock()->GetTranslation() = *inTranslation;
    }
}

static void GetCameraRotation(uint64_t meshInstanceID, glm::vec3* outRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera.lock()) {
        *outRotation = instance.camera.lock()->GetRotation();
    }
}

static void SetCameraRotation(uint64_t meshInstanceID, glm::vec3* inRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera.lock()) {
        instance.camera.lock()->GetRotation() = *inRotation;
    }
}

static void GetCameraDirection(uint64_t meshInstanceID, glm::vec3* outDirection)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera.lock()) {
        *outDirection = instance.camera.lock()->GetDirection();
    }
}

static void SetCameraDirection(uint64_t meshInstanceID, glm::vec3* inDirection)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera.lock()) {
        instance.camera.lock()->GetDirection() = *inDirection;
    }
}

static bool IsKeyDown(Keycode keycode)
{
    return Window::IsKeyDown(keycode);
}

static float GetMouseX()
{
    return Window::GetMousePosNormalizedX();
}

static float GetMouseY()
{
    return Window::GetMousePosNormalizedY();
}

static void ApplyImpulse(uint64_t meshInstanceID, btVector3* impulse)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->applyCentralImpulse(*impulse);
    }
}

static void SetVelocity(uint64_t meshInstanceID, btVector3* velocity)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->setLinearVelocity(*velocity);
    }
}

static void SetAngularVelocity(uint64_t meshInstanceID, btVector3* velocity)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        // TODO: option
        instance.physicsInfo->rigidBodyPtr->setAngularFactor(btVector3(0, 0, 0));
        instance.physicsInfo->rigidBodyPtr->setAngularVelocity(*velocity);
    }
}

void Registry::RegisterFunctions()
{
    mono_add_internal_call("vkApp.InternalCall::GetTranslation", (const void*)GetTranslation);
    mono_add_internal_call("vkApp.InternalCall::SetTranslation", (const void*)SetTranslation);
    mono_add_internal_call("vkApp.InternalCall::GetRotation", (const void*)GetRotation);
    mono_add_internal_call("vkApp.InternalCall::SetRotation", (const void*)SetRotation);
    mono_add_internal_call("vkApp.InternalCall::GetForward", (const void*)GetForward);
    mono_add_internal_call("vkApp.InternalCall::GetRight", (const void*)GetRight);
    mono_add_internal_call("vkApp.InternalCall::GetMatrix", (const void*)GetMatrix);

    mono_add_internal_call("vkApp.InternalCall::GetCameraTranslation", (const void*)GetCameraTranslation);
    mono_add_internal_call("vkApp.InternalCall::SetCameraTranslation", (const void*)SetCameraTranslation);
    mono_add_internal_call("vkApp.InternalCall::GetCameraRotation", (const void*)GetCameraRotation);
    mono_add_internal_call("vkApp.InternalCall::SetCameraRotation", (const void*)SetCameraRotation);
    mono_add_internal_call("vkApp.InternalCall::GetCameraDirection", (const void*)GetCameraDirection);
    mono_add_internal_call("vkApp.InternalCall::SetCameraDirection", (const void*)SetCameraDirection);

    mono_add_internal_call("vkApp.InternalCall::IsKeyDown", (const void*)IsKeyDown);
    mono_add_internal_call("vkApp.InternalCall::GetMouseX", (const void*)GetMouseX);
    mono_add_internal_call("vkApp.InternalCall::GetMouseY", (const void*)GetMouseY);

    mono_add_internal_call("vkApp.InternalCall::ApplyImpulse", (const void*)ApplyImpulse);
    mono_add_internal_call("vkApp.InternalCall::SetVelocity", (const void*)SetVelocity);
    mono_add_internal_call("vkApp.InternalCall::SetAngularVelocity", (const void*)SetAngularVelocity);
}
