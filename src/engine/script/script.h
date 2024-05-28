#ifndef SCRIPT_H
#define SCRIPT_H

#include <cassert>
#include <string>
#include <fstream>
#include <iostream>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include "spdlog/spdlog.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../../path.h"
#include "registry.h"

class Script
{
    friend class ScriptClass;

    inline static MonoDomain* s_rootDomain = nullptr;
    inline static MonoDomain* s_appDomain = nullptr;

    inline static std::shared_ptr<ScriptClass> s_baseScriptClass;
    inline static std::vector<std::shared_ptr<ScriptClass>> s_scriptClasses;
    inline static std::unordered_map<uint64_t, MonoObject*> s_scriptInstances;

    static void InitMono();
    static void LoadAssemblyClasses(MonoAssembly* assembly);

public:
    static void Init();
    static void InvokeMethod(MonoMethod* method, MonoObject* instance, void** params);
    static void Stop();
    static void Reload();
};

class ScriptClass
{
    MonoAssembly* m_monoAssembly = nullptr;
    MonoImage* m_monoImage = nullptr;
    MonoClass* m_monoClass = nullptr;
    std::string m_classNameSpace;
    std::string m_className;

public:
    ScriptClass(MonoAssembly* assembly, const char* nameSpace, const char* name);
    MonoObject* Instantiate();
    MonoMethod* GetMethodByName(const char* name, int paramCount);
    bool IsParentOf(MonoClass* monoClass);
    ~ScriptClass() { std::cout << "Destructor called\n"; }
};

namespace monoUtils {
static char* ReadBytes(const std::string& filepath, uint32_t* outSize);
static MonoAssembly* LoadAssembly(const std::string& assemblyPath);
static void CheckAssemblyTypes(MonoAssembly* assembly);
static void PrintAssemblyTypes(MonoAssembly* assembly);
} // namespace monoUtils

#endif
