#include "registry.h"

static void CppLog_string(MonoString* monoString)
{
    char* cStr = mono_string_to_utf8(monoString);
    std::cout << cStr;
    mono_free(cStr);
}

// static void CppLog_vec3(glm::vec3* param)
// {
//     std::cout << glm::to_string(*param) << std::endl;
// }

static void CppLog_vec3(glm::vec3* inVec, glm::vec3* outVec)
{
    *outVec = *inVec + glm::vec3(1, 2, 3);
}

void Registry::RegisterFunctions()
{
    mono_add_internal_call("vkApp.InternalCalls::CppLog_string", CppLog_string);
    mono_add_internal_call("vkApp.InternalCalls::CppLog_vec3", CppLog_vec3);
}
