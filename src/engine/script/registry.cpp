#include "registry.h"

void Registry::GetTranslation(uint64_t meshInstanceID, glm::vec3* outTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outTranslation = instance.translation;
}

void Registry::SetTranslation(uint64_t meshInstanceID, glm::vec3* inTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    instance.translation = *inTranslation;
    instance.UpdateMatrix();
    Script::s_scene->GetMeshes()[instance.UBO.meshColorID]->UpdateUBO(instance);
}

void Registry::GetRotation(uint64_t meshInstanceID, glm::vec3* outRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outRotation = instance.rotation;
}

void Registry::SetRotation(uint64_t meshInstanceID, glm::vec3* inRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    instance.rotation = *inRotation;
    instance.UpdateMatrix();
    Script::s_scene->GetMeshes()[instance.UBO.meshColorID]->UpdateUBO(instance);
}

void Registry::GetForward(uint64_t meshInstanceID, glm::vec3* outForward)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outForward = glm::normalize(instance.UBO.model * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
}

void Registry::GetRight(uint64_t meshInstanceID, glm::vec3* outRight)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    glm::vec3 forward = glm::normalize(instance.UBO.model * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    *outRight = glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Registry::GetMatrix(uint64_t meshInstanceID, glm::mat4* outMatrix)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    *outMatrix = instance.UBO.model;
}

void Registry::GetCameraTranslation(uint64_t meshInstanceID, glm::vec3* outTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        *outTranslation = instance.camera->GetTranslation();
    }
}

void Registry::SetCameraTranslation(uint64_t meshInstanceID, glm::vec3* inTranslation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        instance.camera->SetTranslation(*inTranslation);
    }
}

void Registry::GetCameraRotation(uint64_t meshInstanceID, glm::vec3* outRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        *outRotation = instance.camera->GetRotation();
    }
}

void Registry::SetCameraRotation(uint64_t meshInstanceID, glm::vec3* inRotation)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        instance.camera->SetRotation(*inRotation);
    }
}

void Registry::GetCameraDirection(uint64_t meshInstanceID, glm::vec3* outDirection)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        *outDirection = instance.camera->GetDirection();
    }
}

void Registry::SetCameraDirection(uint64_t meshInstanceID, glm::vec3* inDirection)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        instance.camera->SetDirection(*inDirection);
    }
}

bool Registry::IsCameraControllable(uint64_t meshInstanceID)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.camera) {
        return instance.camera->IsControllable();
    }
    return false;
}

bool Registry::IsKeyDown(Keycode keycode)
{
    return Window::IsKeyDown(keycode);
}

bool Registry::IsMouseButtonDown(MouseButton mouseButton)
{
    return Window::IsMouseButtonDown(mouseButton);
}

float Registry::GetMouseX()
{
    return Window::GetMousePosNormalizedX();
}

float Registry::GetMouseY()
{
    return Window::GetMousePosNormalizedY();
}

void Registry::SetGravity(uint64_t meshInstanceID, btVector3* acceleration)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->setGravity(*acceleration);
    }
}

void Registry::SetLinearFactor(uint64_t meshInstanceID, btVector3* factor)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->setLinearFactor(*factor);
    }
}

void Registry::SetAngularFactor(uint64_t meshInstanceID, btVector3* factor)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->setAngularFactor(*factor);
    }
}

void Registry::ApplyImpulse(uint64_t meshInstanceID, btVector3* impulse)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->applyCentralImpulse(*impulse);
    }
}

void Registry::GetVelocity(uint64_t meshInstanceID, btVector3* outVelocity)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        *outVelocity = instance.physicsInfo->rigidBodyPtr->getLinearVelocity();
    }
}

void Registry::SetVelocity(uint64_t meshInstanceID, btVector3* inVelocity)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->setLinearVelocity(*inVelocity);
    }
}

void Registry::SetAngularVelocity(uint64_t meshInstanceID, btVector3* velocity)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        instance.physicsInfo->rigidBodyPtr->setAngularVelocity(*velocity);
    }
}

void Registry::SetRigidBodyTransform(uint64_t meshInstanceID, glm::mat4* matrix)
{
    auto& instance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    if (instance.physicsInfo) {
        instance.physicsInfo->rigidBodyPtr->activate(true);
        btTransform transform;
        transform.setFromOpenGLMatrix(glm::value_ptr(*matrix));
        instance.physicsInfo->rigidBodyPtr->setWorldTransform(transform);
    }
}

void Registry::GetRayHitPosition(btVector3* rayFrom, btVector3* rayTo, btVector3* hitPosition)
{
    btCollisionWorld::ClosestRayResultCallback result(*rayFrom, *rayTo);
    Physics::GetDynamicsWorld()->rayTest(*rayFrom, *rayTo, result);
    if (result.hasHit())
        *hitPosition = result.m_hitPointWorld;
}

void Registry::DuplicateMeshInstance(uint64_t meshInstanceID)
{
    auto& meshInstance = Script::s_scene->GetMeshInstanceByID(meshInstanceID);
    Script::s_scene->DuplicateMeshInstance(meshInstance.UBO.meshColorID, meshInstance.UBO.instanceColorID);
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
    mono_add_internal_call("vkApp.InternalCall::IsCameraControllable", (const void*)IsCameraControllable);

    mono_add_internal_call("vkApp.InternalCall::IsMouseButtonDown", (const void*)IsMouseButtonDown);
    mono_add_internal_call("vkApp.InternalCall::IsKeyDown", (const void*)IsKeyDown);
    mono_add_internal_call("vkApp.InternalCall::GetMouseX", (const void*)GetMouseX);
    mono_add_internal_call("vkApp.InternalCall::GetMouseY", (const void*)GetMouseY);

    mono_add_internal_call("vkApp.InternalCall::SetGravity", (const void*)SetGravity);
    mono_add_internal_call("vkApp.InternalCall::SetLinearFactor", (const void*)SetLinearFactor);
    mono_add_internal_call("vkApp.InternalCall::SetAngularFactor", (const void*)SetAngularFactor);
    mono_add_internal_call("vkApp.InternalCall::ApplyImpulse", (const void*)ApplyImpulse);
    mono_add_internal_call("vkApp.InternalCall::GetVelocity", (const void*)GetVelocity);
    mono_add_internal_call("vkApp.InternalCall::SetVelocity", (const void*)SetVelocity);
    mono_add_internal_call("vkApp.InternalCall::SetAngularVelocity", (const void*)SetAngularVelocity);
    mono_add_internal_call("vkApp.InternalCall::SetRigidBodyTransform", (const void*)SetRigidBodyTransform);
    mono_add_internal_call("vkApp.InternalCall::GetRayHitPosition", (const void*)GetRayHitPosition);

    mono_add_internal_call("vkApp.InternalCall::DuplicateMeshInstance", (const void*)DuplicateMeshInstance);
}
