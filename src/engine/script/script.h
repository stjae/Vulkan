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
#include "../../scene/mesh.h"
#include "../../scene/scene.h"
#include "../sceneSerializer.h"

class ScriptClass;
class ScriptInstance;

class Script
{
    friend class UI;
    friend class Scene;
    friend class SceneSerializer;

    inline static MonoDomain* s_rootDomain = nullptr;
    inline static std::unordered_map<std::string, std::shared_ptr<ScriptClass>> s_scriptClasses;
    inline static std::unordered_map<uint64_t, std::shared_ptr<ScriptInstance>> s_scriptInstances;

    static void InitMono();
    static void LoadAssemblyClasses(const std::string& filePath);

public:
    inline static Scene* s_scene;
    inline static MonoDomain* s_appDomain = nullptr;
    inline static std::unique_ptr<ScriptClass> s_baseScriptClass;

    static void Init(Scene* scene);
    static void LoadDll(const std::string& sceneFolderPath);
    static void InvokeMethod(MonoMethod* method, MonoObject* instance, void** params);
    static std::string GetScriptClassName(uint64_t UUID);
    static bool InstanceExists(uint64_t UUID);
    static void Reset();
};

class ScriptClass
{
    friend Script;
    friend SceneSerializer;

    MonoImage* m_monoImage = nullptr;
    MonoClass* m_monoClass = nullptr;
    std::string m_classNameSpace;
    std::string m_className;
    std::string m_fullName;
    std::string m_filePath;
    std::string m_relativePath;

public:
    ScriptClass(const char* nameSpace, const char* name, const std::string& filePath);
    void Init();
    MonoObject* Instantiate();
    MonoMethod* GetMethodByName(const char* name, int paramCount);
    bool IsParentOf(MonoClass* monoClass);
    const std::string& GetName() { return m_fullName; }
};

class ScriptInstance
{
    friend Script;
    friend Scene;
    friend SceneSerializer;

    std::shared_ptr<ScriptClass> m_scriptClass;
    MonoObject* m_instance = nullptr;
    MonoMethod* m_constructor = nullptr;
    MonoMethod* m_onCreateMethod = nullptr;
    MonoMethod* m_onUpdateMethod = nullptr;
    MonoMethod* m_onDestroyMethod = nullptr;
    uint64_t m_meshInstanceID;

public:
    ScriptInstance(std::shared_ptr<ScriptClass>& scriptClass, uint64_t meshInstanceID);
    void Init();
    void InvokeOnCreate();
    void InvokeOnUpdate(float dt);
    void InvokeOnDestroy();
};

namespace monoUtils {
static char* ReadBytes(const std::string& filepath, uint32_t* outSize);
static MonoAssembly* LoadAssembly(const std::string& assemblyPath);
static void PrintAssemblyTypes(MonoAssembly* assembly);
} // namespace monoUtils

#endif
