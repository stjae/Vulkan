#include "script.h"
#include <utility>
#include "registry.h"
#include <filesystem>

void Script::Init(Scene* scene)
{
    s_scene = scene;
    InitMono();
    Registry::RegisterFunctions();

    // Base
    s_baseScriptClass = std::make_unique<ScriptClass>("vkApp", "MeshInstance", "vkApp.dll");
    s_baseScriptClass->Init();
}

void Script::LoadDll(const std::string& sceneFolderPath)
{
    for (auto& dir : std::filesystem::recursive_directory_iterator(sceneFolderPath)) {
        if (dir.path().extension() == ".dll") {
            Log(DEBUG, fmt::terminal_color::white, "found dll: {0}", dir.path().string());
            MonoAssembly* loadedAssembly = monoUtils::LoadAssembly(dir.path().string());
            if (loadedAssembly != nullptr)
                LoadAssemblyClasses(dir.path().string());
        }
    }
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

void Script::Reset()
{
    s_appDomain = mono_domain_create_appdomain("vkAppDomain", nullptr);
    mono_domain_set(s_appDomain, true);

    s_baseScriptClass = std::make_unique<ScriptClass>("vkApp", "MeshInstance", "vkApp.dll");
    s_baseScriptClass->Init();
}

void Script::LoadAssemblyClasses(const std::string& filePath)
{
    MonoAssembly* assembly = monoUtils::LoadAssembly(filePath);
    if (!assembly) {
        spdlog::error("{} was not found!", filePath);
        return;
    }
    MonoImage* image = mono_assembly_get_image(assembly);
    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

    for (int32_t i = 0; i < numTypes; i++) {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

        MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

        if (monoClass == nullptr) {
            continue;
        }

        if (s_baseScriptClass->IsParentOf(monoClass)) {
            auto scriptClass = std::make_shared<ScriptClass>(nameSpace, name, filePath);
            s_scriptClasses.emplace(scriptClass->m_fullName, scriptClass);
            std::string dllPath = filePath.substr(0, filePath.find_last_of("/\\") + 1);
            auto copyOptions = std::filesystem::copy_options::update_existing;
            std::error_code ec;
            std::filesystem::copy("vkApp.dll", dllPath, copyOptions, ec);
            Log(DEBUG, fmt::terminal_color::bright_black, "copy reference script dll: {0}, {1}", "vkApp", ec.message());
            std::filesystem::copy("System.Numerics.dll", dllPath, copyOptions, ec);
            Log(DEBUG, fmt::terminal_color::bright_black, "copy reference script dll: {0}, {1}", "System.Numerics.dll", ec.message());
        }
    }
}

void Script::InvokeMethod(MonoMethod* method, MonoObject* instance, void** params)
{
    mono_runtime_invoke(method, instance, params, nullptr);
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

ScriptClass::ScriptClass(const char* nameSpace, const char* name, const std::string& filePath)
    : m_classNameSpace(nameSpace), m_className(name), m_filePath(filePath)
{
    m_fullName = m_classNameSpace.empty() ? m_className : m_classNameSpace + "::" + m_className;
}

void ScriptClass::Init()
{
    MonoAssembly* assembly = monoUtils::LoadAssembly(m_filePath);
    m_monoImage = mono_assembly_get_image(assembly);
    m_monoClass = mono_class_from_name(m_monoImage, m_classNameSpace.c_str(), m_className.c_str());
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
    if (monoClass == m_monoClass)
        return false;
    return mono_class_is_subclass_of(monoClass, m_monoClass, true);
}

ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass>& scriptClass, uint64_t meshInstanceID)
    : m_scriptClass(scriptClass), m_meshInstanceID(meshInstanceID) {}
void ScriptInstance::Init()
{
    m_instance = m_scriptClass->Instantiate();
    m_constructor = Script::s_baseScriptClass->GetMethodByName(".ctor", 1);
    m_onCreateMethod = m_scriptClass->GetMethodByName("OnCreate", 0);
    m_onUpdateMethod = m_scriptClass->GetMethodByName("OnUpdate", 1);
    m_onDestroyMethod = m_scriptClass->GetMethodByName("OnDestroy", 0);

    // call constructor
    void* param = &m_meshInstanceID;
    if (m_instance)
        Script::InvokeMethod(m_constructor, m_instance, &param);
}
void ScriptInstance::InvokeOnCreate()
{
    if (m_instance && m_onCreateMethod)
        Script::InvokeMethod(m_onCreateMethod, m_instance, nullptr);
}

void ScriptInstance::InvokeOnUpdate(float dt)
{
    void* param = &dt;
    if (m_instance && m_onUpdateMethod)
        Script::InvokeMethod(m_onUpdateMethod, m_instance, &param);
}

void ScriptInstance::InvokeOnDestroy()
{
    if (m_instance && m_onDestroyMethod)
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
