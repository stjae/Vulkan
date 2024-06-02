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
    *outForward = glm::normalize(instance.UBO.model * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
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

static void GetCameraRotation(uint64_t meshInstanceID, glm::vec3* outRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        *outRotation = instance.camera->GetRotation();
    }
}

static void SetCameraRotation(uint64_t meshInstanceID, glm::vec3* inRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        instance.camera->GetRotation() = *inRotation;
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
    mono_add_internal_call("vkApp.InternalCall::GetTranslation", GetTranslation);
    mono_add_internal_call("vkApp.InternalCall::SetTranslation", SetTranslation);
    mono_add_internal_call("vkApp.InternalCall::GetRotation", GetRotation);
    mono_add_internal_call("vkApp.InternalCall::SetRotation", SetRotation);
    mono_add_internal_call("vkApp.InternalCall::GetForward", GetForward);
    mono_add_internal_call("vkApp.InternalCall::GetRight", GetRight);
    mono_add_internal_call("vkApp.InternalCall::GetMatrix", GetMatrix);

    mono_add_internal_call("vkApp.InternalCall::GetCameraRotation", GetCameraRotation);
    mono_add_internal_call("vkApp.InternalCall::SetCameraRotation", SetCameraRotation);

    mono_add_internal_call("vkApp.InternalCall::IsKeyDown", IsKeyDown);
    mono_add_internal_call("vkApp.InternalCall::GetMouseX", GetMouseX);
    mono_add_internal_call("vkApp.InternalCall::GetMouseY", GetMouseY);

    mono_add_internal_call("vkApp.InternalCall::ApplyImpulse", ApplyImpulse);
    mono_add_internal_call("vkApp.InternalCall::SetVelocity", SetVelocity);
    mono_add_internal_call("vkApp.InternalCall::SetAngularVelocity", SetAngularVelocity);
}
