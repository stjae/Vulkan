#include "script.h"

#include <utility>
#include "registry.h"
#include "../../scene/scene.h"

void Script::Init(Scene* scene)
{
    s_scene = scene;
    InitMono();
    Registry::RegisterFunctions();

    // Base
    MonoAssembly* baseAssembly = monoUtils::LoadAssembly(PROJECT_DIR "script/vkApp.dll");
    s_baseScriptClass = std::make_shared<ScriptClass>(baseAssembly, "vkApp", "MeshInstance");

    // Player
    MonoAssembly* playerAssembly = monoUtils::LoadAssembly(PROJECT_DIR "script/player.dll");
    LoadAssemblyClasses(playerAssembly);
}

void Script::InitMono()
{
    mono_set_assemblies_path("mono/lib");

    MonoDomain* rootDomain = mono_jit_init("vkScriptRuntime");
    assert(rootDomain != nullptr);

    s_rootDomain = rootDomain;

    s_appDomain = mono_domain_create_appdomain("vkAppDomain", nullptr);
    mono_domain_set(s_appDomain, true);
}

void Script::LoadAssemblyClasses(MonoAssembly* assembly)
{
    MonoImage* image = mono_assembly_get_image(assembly);
    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    for (int32_t i = 0; i < numTypes; i++) {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

        MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

        if (s_baseScriptClass->IsParentOf(monoClass)) {
            s_scriptClasses.emplace_back(std::make_shared<ScriptClass>(assembly, nameSpace, name));
        }
    }
}

void Script::InvokeMethod(MonoMethod* method, MonoObject* instance, void** params)
{
    mono_runtime_invoke(method, instance, params, nullptr);
}

void Script::Reload()
{
    s_appDomain = mono_domain_create_appdomain("vkAppDomain", nullptr);
    mono_domain_set(s_appDomain, true);

    s_scriptClasses.clear();
    for (auto& scriptClass : s_scriptClasses) {
        scriptClass.reset();
    }

    MonoAssembly* baseAssembly = monoUtils::LoadAssembly(PROJECT_DIR "script/vkApp.dll");
    s_baseScriptClass = std::make_shared<ScriptClass>(baseAssembly, "vkApp", "MeshInstance");

    MonoAssembly* playerAssembly = monoUtils::LoadAssembly("C:/Users/stjae/Desktop/player/bin/Debug/player.dll");
    LoadAssemblyClasses(playerAssembly);

    auto& sc = s_scriptClasses;
    MonoObject* playerInstance = s_scriptClasses[0]->Instantiate();
    MonoMethod* printFunc = s_scriptClasses[0]->GetMethodByName("OnCreate", 0);
    InvokeMethod(printFunc, playerInstance, nullptr);
}

std::string Script::GetScriptClassName(uint64_t UUID)
{
    std::string className = "None";
    if (s_scriptInstances.find(UUID) != s_scriptInstances.end())
        return s_scriptInstances[UUID]->m_scriptClass->m_fullName;
    else
        return className;
}
bool Script::InstanceExists(uint64_t UUID)
{
    if (s_scriptInstances.find(UUID) != s_scriptInstances.end())
        return true;
    else
        return false;
}

// ScriptClass
ScriptClass::ScriptClass(MonoAssembly* assembly, const char* nameSpace, const char* name)
    : m_monoAssembly(assembly), m_classNameSpace(nameSpace), m_className(name)
{
    m_monoImage = mono_assembly_get_image(assembly);
    m_monoClass = mono_class_from_name(m_monoImage, m_classNameSpace.c_str(), m_className.c_str());
    m_fullName = m_classNameSpace.empty() ? m_className : m_classNameSpace + "::" + m_className;
}
MonoObject* ScriptClass::Instantiate()
{
    MonoObject* monoInstance = mono_object_new(Script::s_appDomain, m_monoClass);
    mono_runtime_object_init(monoInstance);
    return monoInstance;
}
MonoMethod* ScriptClass::GetMethodByName(const char* name, int paramCount)
{
    return mono_class_get_method_from_name(m_monoClass, name, paramCount);
}

bool ScriptClass::IsParentOf(MonoClass* monoClass)
{
    return mono_class_is_subclass_of(monoClass, m_monoClass, true);
}

// ScriptInstance
ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass>& scriptClass, MeshInstance& meshInstance) : m_scriptClass(scriptClass)
{
    m_instance = scriptClass->Instantiate();
    m_constructor = Script::s_baseScriptClass->GetMethodByName(".ctor", 1);
    m_onCreateMethod = scriptClass->GetMethodByName("OnCreate", 0);
    m_onUpdateMethod = scriptClass->GetMethodByName("OnUpdate", 1);
    m_onDestroyMethod = scriptClass->GetMethodByName("OnDestroy", 0);

    // call constructor
    uint64_t meshInstanceID = meshInstance.UUID;
    void* param = &meshInstanceID;
    Script::InvokeMethod(m_constructor, m_instance, &param);
}

void ScriptInstance::InvokeOnCreate()
{
    Script::InvokeMethod(m_onCreateMethod, m_instance, nullptr);
}

void ScriptInstance::InvokeOnUpdate(float dt)
{
    void* param = &dt;
    Script::InvokeMethod(m_onUpdateMethod, m_instance, &param);
}

void ScriptInstance::InvokeOnDestroy()
{
    Script::InvokeMethod(m_onDestroyMethod, m_instance, nullptr);
}

namespace monoUtils {
char* ReadBytes(const std::string& filepath, uint32_t* outSize)
{
    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

    if (!stream) {
        spdlog::error("Failed to load script binary from \"{}\"", filepath);
        return nullptr;
    }

    std::streampos end = stream.tellg();
    stream.seekg(0, std::ios::beg);
    uint32_t size = end - stream.tellg();

    if (size == 0) {
        spdlog::error("Size of script binary of \"{}\" is 0", filepath);
        return nullptr;
    }

    char* buffer = new char[size];
    stream.read((char*)buffer, size);
    stream.close();

    *outSize = size;
    return buffer;
}
MonoAssembly* LoadAssembly(const std::string& assemblyPath)
{
    uint32_t fileSize = 0;
    char* fileData = ReadBytes(assemblyPath, &fileSize);

    MonoImageOpenStatus status;
    MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

    if (status != MONO_IMAGE_OK) {
        const char* errorMessage = mono_image_strerror(status);
        spdlog::error(errorMessage);
        return nullptr;
    }

    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
    mono_image_close(image);

    delete[] fileData;

    return assembly;
}
void PrintAssemblyTypes(MonoAssembly* assembly)
{
    MonoImage* image = mono_assembly_get_image(assembly);
    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    for (int32_t i = 0; i < numTypes; i++) {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

        printf("%s.%s\n", nameSpace, name);
    }
}
} // namespace monoUtils
