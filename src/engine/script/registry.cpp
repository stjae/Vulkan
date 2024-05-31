#include "registry.h"
#include "script.h"
#include "../../scene/scene.h"

static void CppLog_string(MonoString* monoString)
{
    char* cStr = mono_string_to_utf8(monoString);
    std::cout << cStr;
    mono_free(cStr);
}

static void CppLog_vec3(glm::vec3* param)
{
    std::cout << glm::to_string(*param) << std::endl;
}

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

void Registry::RegisterFunctions()
{
    mono_add_internal_call("vkApp.InternalCalls::CppLog_string", CppLog_string);
    mono_add_internal_call("vkApp.InternalCalls::CppLog_vec3", CppLog_vec3);
    mono_add_internal_call("vkApp.InternalCalls::GetTranslation", GetTranslation);
    mono_add_internal_call("vkApp.InternalCalls::SetTranslation", SetTranslation);
}
