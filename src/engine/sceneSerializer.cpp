#include "sceneSerializer.h"
#include "script/script.h"

namespace YAML {
template <>
struct convert<glm::vec3>
{
    static bool decode(const Node& node, glm::vec3& rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};
template <>
struct convert<glm::vec4>
{
    static bool decode(const Node& node, glm::vec4& rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};
template <>
struct convert<glm::mat4>
{
    static bool decode(const Node& node, glm::mat4& rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
            return false;

        rhs[0] = node[0].as<glm::vec4>();
        rhs[1] = node[1].as<glm::vec4>();
        rhs[2] = node[2].as<glm::vec4>();
        rhs[3] = node[3].as<glm::vec4>();
        return true;
    }
};
} // namespace YAML

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& vec)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << vec.x << vec.y << vec.z << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& vec)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << vec.x << vec.y << vec.z << vec.w << YAML::EndSeq;
    return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& mat)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << mat[0] << mat[1] << mat[2] << mat[3] << YAML::EndSeq;
    return out;
}

void SceneSerializer::Serialize(const Scene& scene)
{
    YAML::Emitter out;

    out << YAML::BeginMap;

    out << YAML::Key << "GridWidth" << YAML::Value << scene.m_gridWidth;
    out << YAML::Key << "ShowGrid" << YAML::Value << scene.m_showGrid;

    if (!scene.m_hdriFilePath.empty()) {
        out << YAML::Key << "HDRIFilePath" << YAML::Value << scene.m_hdriFilePath;
    }
    out << YAML::Key << "IBLExposure" << YAML::Value << scene.m_iblExposure;

    SerializeDirLight(out, scene);
    SerializePointLight(out, scene);
    SerializeCamera(out, scene);
    SerializeResource(out, scene);
    SerializeMesh(out, scene);
    SerializeScriptClass(out, scene);
    SerializeScriptInstance(out);

    out << YAML::EndMap;

    std::ofstream fout(scene.m_sceneFilePath);
    fout << out.c_str();
}

void SceneSerializer::SerializeDirLight(YAML::Emitter& out, const Scene& scene)
{
    out << YAML::Key << "DirectionalLight" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Position" << YAML::Value << scene.m_dirLight.pos;
    out << YAML::Key << "Color" << YAML::Value << scene.m_dirLight.color;
    out << YAML::Key << "Intensity" << YAML::Value << scene.m_dirLight.intensity;
    out << YAML::EndMap;
}

void SceneSerializer::SerializePointLight(YAML::Emitter& out, const Scene& scene)
{
    if (scene.m_pointLight.Size() > 0) {
        out << YAML::Key << "PointLight";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& UBO : scene.m_pointLight.GetUBOs()) {
            out << YAML::BeginMap;
            out << YAML::Key << "Position" << YAML::Value << UBO.pos;
            out << YAML::Key << "Color" << YAML::Value << UBO.color;
            out << YAML::Key << "Intensity" << YAML::Value << UBO.intensity;
            out << YAML::Key << "Range" << YAML::Value << UBO.range;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeCamera(YAML::Emitter& out, const Scene& scene)
{
    out << YAML::Key << "Camera" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "PlayCameraID" << YAML::Value << scene.m_playCamera->GetID();
    out << YAML::Key << "Position" << YAML::Value << scene.m_mainCamera.m_pos;
    out << YAML::Key << "Direction" << YAML::Value << scene.m_mainCamera.m_dir;
    out << YAML::Key << "CascadeRange1" << YAML::Value << Camera::s_cascadeRanges[0];
    out << YAML::Key << "CascadeRange2" << YAML::Value << Camera::s_cascadeRanges[1];
    out << YAML::Key << "CascadeRange3" << YAML::Value << Camera::s_cascadeRanges[2];
    out << YAML::Key << "CascadeRange4" << YAML::Value << Camera::s_cascadeRanges[3];
    out << YAML::Key << "UseMotionBlur" << YAML::Value << postProcessPushConstants.useMotionBlur;
    out << YAML::Key << "Divisor" << YAML::Value << postProcessPushConstants.divisor;
    out << YAML::EndMap;
}

void SceneSerializer::SerializeResource(YAML::Emitter& out, const Scene& scene)
{
    if (!scene.m_resources.empty()) {
        out << YAML::Key << "Resource";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& resource : scene.m_resources) {
            out << YAML::BeginMap;
            out << YAML::Key << "FilePath" << YAML::Value << resource.filePath;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeMesh(YAML::Emitter& out, const Scene& scene)
{
    if (!scene.m_meshes.empty()) {
        out << YAML::Key << "Mesh";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& mesh : scene.m_meshes) {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << mesh->m_name;
            out << YAML::Key << "Instance";
            out << YAML::Value << YAML::BeginSeq;
            for (const auto& instance : mesh->GetInstances()) {
                out << YAML::BeginMap;
                out << YAML::Key << "UUID" << YAML::Value << instance->UUID;
                out << YAML::Key << "Translation" << YAML::Value << instance->translation;
                out << YAML::Key << "Rotation" << YAML::Value << instance->rotation;
                out << YAML::Key << "Scale" << YAML::Value << instance->scale;
                out << YAML::Key << "Transform" << YAML::Value << instance->UBO.model;
                out << YAML::Key << "UseAlbedoTexture" << YAML::Value << instance->UBO.useAlbedoTexture;
                out << YAML::Key << "UseNormalTexture" << YAML::Value << instance->UBO.useNormalTexture;
                out << YAML::Key << "UseMetallicTexture" << YAML::Value << instance->UBO.useMetallicTexture;
                out << YAML::Key << "UseRoughnessTexture" << YAML::Value << instance->UBO.useRoughnessTexture;
                out << YAML::Key << "Albedo" << YAML::Value << instance->UBO.albedo;
                out << YAML::Key << "Metallic" << YAML::Value << instance->UBO.metallic;
                out << YAML::Key << "Roughness" << YAML::Value << instance->UBO.roughness;
                if (instance->physicsInfo) {
                    out << YAML::Key << "PhysicsInfo" << YAML::Value;
                    out << YAML::BeginMap;
                    out << YAML::Key << "Type" << YAML::Value << (int)instance->physicsInfo->rigidBodyType;
                    out << YAML::Key << "Shape" << YAML::Value << (int)instance->physicsInfo->colliderShape;
                    out << YAML::Key << "RigidBodyScale" << YAML::Value << instance->physicsInfo->scale;
                    out << YAML::EndMap;
                }
                if (instance->camera) {
                    out << YAML::Key << "Camera" << YAML::Value << true;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeScriptClass(YAML::Emitter& out, const Scene& scene)
{
    if (!Script::s_scriptClasses.empty()) {
        out << YAML::Key << "ScriptClass";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& scriptClass : Script::s_scriptClasses) {
            out << YAML::BeginMap;
            auto& filePath = scriptClass.second->m_filePath;
            auto relativePath = filePath.substr(scene.m_sceneFolderPath.length(), filePath.length());
            out << YAML::Key << "ScriptFilePath" << YAML::Value << relativePath;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeScriptInstance(YAML::Emitter& out)
{
    if (!Script::s_scriptInstances.empty()) {
        out << YAML::Key << "ScriptInstance";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& instance : Script::s_scriptInstances) {
            out << YAML::BeginMap;
            out << YAML::Key << "ClassName" << YAML::Value << instance.second->m_scriptClass->m_fullName;
            out << YAML::Key << "MeshInstanceID" << YAML::Value << instance.first;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::Deserialize(Scene& scene, const std::string& filePath)
{
    scene.Clear();

    std::ifstream stream(filePath);
    std::stringstream strStream;
    strStream << stream.rdbuf();

    YAML::Node data = YAML::Load(strStream.str());

    scene.m_sceneFolderPath = filePath.substr(0, filePath.find_last_of("/\\"));
    scene.m_sceneFilePath = filePath;

    auto camera = data["Camera"];
    scene.m_mainCamera.m_pos = camera["Position"].as<glm::vec3>();
    scene.m_mainCamera.m_dir = camera["Direction"].as<glm::vec3>();
    Camera::s_cascadeRanges[0] = camera["CascadeRange1"].as<float>();
    Camera::s_cascadeRanges[1] = camera["CascadeRange2"].as<float>();
    Camera::s_cascadeRanges[2] = camera["CascadeRange3"].as<float>();
    Camera::s_cascadeRanges[3] = camera["CascadeRange4"].as<float>();
    scene.m_mainCamera.m_at = scene.m_mainCamera.m_pos + scene.m_mainCamera.m_dir;
    postProcessPushConstants.useMotionBlur = camera["UseMotionBlur"].as<int>();
    postProcessPushConstants.divisor = camera["Divisor"].as<float>();

    auto hdriFilePath = data["HDRIFilePath"];
    if (hdriFilePath) {
        scene.AddEnvironmentMap(hdriFilePath.as<std::string>());
        scene.m_hdriFilePath = hdriFilePath.as<std::string>();
    } else {
        scene.SelectDummyEnvMap();
    }

    auto iblExposure = data["IBLExposure"];
    if (iblExposure) {
        scene.m_iblExposure = iblExposure.as<float>();
    }

    auto dirLight = data["DirectionalLight"];
    scene.m_dirLight.pos = dirLight["Position"].as<glm::vec3>();
    scene.m_dirLight.color = dirLight["Color"].as<glm::vec3>();
    scene.m_dirLight.intensity = dirLight["Intensity"].as<float>();

    auto pointLights = data["PointLight"];
    if (pointLights) {
        for (auto pointLight : pointLights) {
            scene.AddPointLight();
            scene.m_pointLight.m_UBOs.back().pos = pointLight["Position"].as<glm::vec3>();
            scene.m_pointLight.m_UBOs.back().color = pointLight["Color"].as<glm::vec3>();
            scene.m_pointLight.m_UBOs.back().intensity = pointLight["Intensity"].as<float>();
            scene.m_pointLight.m_UBOs.back().range = pointLight["Range"].as<float>();
        }
    }

    auto resources = data["Resource"];
    if (resources) {
        for (auto resource : resources) {
            auto relativePath = resource["FilePath"].as<std::string>();
            auto fullPath = scene.m_sceneFolderPath + relativePath;
            scene.AddResource(fullPath);
        }
    }

    auto meshes = data["Mesh"];
    if (meshes) {
        for (size_t i = 0; i < meshes.size(); i++) {
            auto instances = meshes[i]["Instance"];
            for (auto instance : instances) {
                scene.AddMeshInstance(*scene.m_meshes[i], instance["UUID"].as<uint64_t>());
                auto& meshInstance = scene.m_meshes[i]->GetInstances().back();
                meshInstance->translation = instance["Translation"].as<glm::vec3>();
                meshInstance->rotation = instance["Rotation"].as<glm::vec3>();
                meshInstance->scale = instance["Scale"].as<glm::vec3>();
                meshInstance->UBO.model = instance["Transform"].as<glm::mat4>();
                meshInstance->UBO.useAlbedoTexture = instance["UseAlbedoTexture"].as<int32_t>();
                meshInstance->UBO.useNormalTexture = instance["UseNormalTexture"].as<int32_t>();
                meshInstance->UBO.useMetallicTexture = instance["UseMetallicTexture"].as<int32_t>();
                meshInstance->UBO.useRoughnessTexture = instance["UseRoughnessTexture"].as<int32_t>();
                meshInstance->UBO.albedo = instance["Albedo"].as<glm::vec3>();
                meshInstance->UBO.metallic = instance["Metallic"].as<float>();
                meshInstance->UBO.roughness = instance["Roughness"].as<float>();
                auto physicsInfo = instance["PhysicsInfo"];
                if (physicsInfo) {
                    PhysicsInfo pInfo;
                    pInfo.rigidBodyType = (eRigidBodyType)physicsInfo["Type"].as<int>();
                    pInfo.colliderShape = (eColliderShape)physicsInfo["Shape"].as<int>();
                    scene.AddPhysics(*scene.m_meshes[i], *meshInstance, pInfo);
                    meshInstance->physicsInfo->scale = physicsInfo["RigidBodyScale"].as<glm::vec3>();
                }
                auto subCamera = instance["Camera"];
                if (subCamera) {
                    scene.AddCamera(*meshInstance);
                    if (camera["PlayCameraID"].as<uint64_t>() == meshInstance->UUID)
                        scene.m_playCamera = meshInstance->camera.get();
                }
            }
        }
        scene.UpdateMeshBuffer();
    }

    auto scriptClasses = data["ScriptClass"];
    if (scriptClasses) {
        for (auto&& scriptClass : scriptClasses) {
            Script::LoadAssemblyClasses(scene.m_sceneFolderPath + scriptClass["ScriptFilePath"].as<std::string>());
        }
    }

    auto scriptInstances = data["ScriptInstance"];
    if (scriptInstances) {
        for (auto&& scriptInstance : scriptInstances) {
            auto scriptClassName = scriptInstance["ClassName"].as<std::string>();
            auto meshInstanceID = scriptInstance["MeshInstanceID"].as<uint64_t>();

            // Add script instance only if script class was successfully loaded
            if (Script::s_scriptClasses.find(scriptClassName) != Script::s_scriptClasses.end())
                Script::s_scriptInstances.emplace(meshInstanceID, std::make_shared<ScriptInstance>(Script::s_scriptClasses[scriptClassName], meshInstanceID));
        }
    }

    scene.m_gridWidth = data["GridWidth"].as<int>();
    scene.m_showGrid = data["ShowGrid"].as<bool>();
}
