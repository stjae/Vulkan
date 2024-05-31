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
#include "../../scene/mesh.h"

class ScriptClass;
class ScriptInstance;

class Script
{
    friend class UI;
    friend class Scene;

    inline static MonoDomain* s_rootDomain = nullptr;
    inline static std::vector<std::shared_ptr<ScriptClass>> s_scriptClasses;
    inline static std::unordered_map<uint64_t, std::shared_ptr<ScriptInstance>> s_scriptInstances;

    static void InitMono();
    static void LoadAssemblyClasses(MonoAssembly* assembly);

public:
    inline static Scene* s_scene;
    inline static MonoDomain* s_appDomain = nullptr;
    inline static std::shared_ptr<ScriptClass> s_baseScriptClass;

    static void Init(Scene* scene);
    static void InvokeMethod(MonoMethod* method, MonoObject* instance, void** params);
    static void Reload();
    static std::string GetScriptClassName(uint64_t UUID);
    static bool InstanceExists(uint64_t UUID);
};

class ScriptClass
{
    friend Script;

    MonoAssembly* m_monoAssembly = nullptr;
    MonoImage* m_monoImage = nullptr;
    MonoClass* m_monoClass = nullptr;
    std::string m_classNameSpace;
    std::string m_className;
    std::string m_fullName;

public:
    ScriptClass(MonoAssembly* assembly, const char* nameSpace, const char* name);
    MonoObject* Instantiate();
    MonoMethod* GetMethodByName(const char* name, int paramCount);
    bool IsParentOf(MonoClass* monoClass);
    const std::string& GetName() { return m_fullName; }
};

class ScriptInstance
{
    friend Script;
    friend Scene;

    std::shared_ptr<ScriptClass> m_scriptClass;
    MonoObject* m_instance = nullptr;
    MonoMethod* m_constructor = nullptr;
    MonoMethod* m_onCreateMethod = nullptr;
    MonoMethod* m_onUpdateMethod = nullptr;

public:
    ScriptInstance(std::shared_ptr<ScriptClass>& scriptClass, MeshInstance& meshInstance);
    void InvokeOnCreate();
    void InvokeOnUpdate(float dt);
};

namespace monoUtils {
static char* ReadBytes(const std::string& filepath, uint32_t* outSize);
static MonoAssembly* LoadAssembly(const std::string& assemblyPath);
static void CheckAssemblyTypes(MonoAssembly* assembly);
static void PrintAssemblyTypes(MonoAssembly* assembly);
} // namespace monoUtils

#endif
