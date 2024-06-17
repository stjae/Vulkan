#include "sceneSerializer.h"

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

    out << YAML::Key << "SceneFolderPath" << YAML::Value << scene.m_sceneFolderPath;
    out << YAML::Key << "SceneFilePath" << YAML::Value << scene.m_sceneFilePath;
    if (!scene.m_hdriFilePath.empty()) {
        out << YAML::Key << "HDRIFilePath" << YAML::Value << scene.m_hdriFilePath;
    }
    out << YAML::Key << "IBLExposure" << YAML::Value << scene.m_iblExposure;

    SerializeDirLight(out, scene);
    SerializePointLight(out, scene.m_pointLight);
    SerializeCamera(out, scene.m_mainCamera, scene.m_subCameras);
    SerializeResource(out, scene.m_resources);
    SerializeMesh(out, scene.m_meshes);
    SerializeScriptClass(out);
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

void SceneSerializer::SerializePointLight(YAML::Emitter& out, const PointLight& pointLight)
{
    if (pointLight.Size() > 0) {
        out << YAML::Key << "PointLight";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& UBO : pointLight.Get()) {
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

void SceneSerializer::SerializeCamera(YAML::Emitter& out, const Camera& camera, const std::unordered_map<uint64_t, std::shared_ptr<SubCamera>>& subCameras)
{
    out << YAML::Key << "Camera" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Position" << YAML::Value << camera.m_pos;
    out << YAML::Key << "Dir" << YAML::Value << camera.m_dir;
    out << YAML::Key << "Cascade Range 1" << YAML::Value << camera.m_cascadeRanges[0];
    out << YAML::Key << "Cascade Range 2" << YAML::Value << camera.m_cascadeRanges[1];
    out << YAML::Key << "Cascade Range 3" << YAML::Value << camera.m_cascadeRanges[2];
    out << YAML::Key << "Cascade Range 4" << YAML::Value << camera.m_cascadeRanges[3];
    out << YAML::EndMap;

    // if (!subCameras.empty()) {
    //     out << YAML::Key << "Sub Camera";
    //     out << YAML::Value << YAML::BeginSeq;
    //     for (auto& subCamera : subCameras) {
    //         out << YAML::BeginMap;
    //         out << YAML::Key << "Mesh Instance ID" << YAML::Value << subCamera.second->m_assignedMeshInstanceID;
    //         out << YAML::Key << "Cascade Range 1" << YAML::Value << subCamera.second->m_cascadeRanges[0];
    //         out << YAML::Key << "Cascade Range 2" << YAML::Value << subCamera.second->m_cascadeRanges[1];
    //         out << YAML::Key << "Cascade Range 3" << YAML::Value << subCamera.second->m_cascadeRanges[2];
    //         out << YAML::Key << "Cascade Range 4" << YAML::Value << subCamera.second->m_cascadeRanges[3];
    //         out << YAML::EndMap;
    //     }
    //     out << YAML::EndSeq;
    // }
}

void SceneSerializer::SerializeResource(YAML::Emitter& out, const std::vector<Resource>& resources)
{
    if (!resources.empty()) {
        out << YAML::Key << "Resource";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& resource : resources) {
            out << YAML::BeginMap;
            out << YAML::Key << "FilePath" << YAML::Value << resource.filePath;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeMesh(YAML::Emitter& out, const std::vector<std::shared_ptr<Mesh>>& meshes)
{
    if (!meshes.empty()) {
        out << YAML::Key << "Mesh";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& mesh : meshes) {
            out << YAML::BeginMap;
            out << YAML::Key << "Instance";
            out << YAML::Value << YAML::BeginSeq;
            for (const auto& instance : mesh->GetInstances()) {
                out << YAML::BeginMap;
                out << YAML::Key << "UUID" << YAML::Value << instance->UUID;
                out << YAML::Key << "Translation" << YAML::Value << instance->translation;
                out << YAML::Key << "Rotation" << YAML::Value << instance->rotation;
                out << YAML::Key << "Scale" << YAML::Value << instance->scale;
                out << YAML::Key << "Transform" << YAML::Value << instance->UBO.model;
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
                if (instance->camera.lock()) {
                    out << YAML::Key << "Camera" << YAML::Value;
                    out << YAML::BeginMap;
                    out << YAML::Key << "Cascade Range 1" << YAML::Value << instance->camera.lock()->m_cascadeRanges[0];
                    out << YAML::Key << "Cascade Range 2" << YAML::Value << instance->camera.lock()->m_cascadeRanges[1];
                    out << YAML::Key << "Cascade Range 3" << YAML::Value << instance->camera.lock()->m_cascadeRanges[2];
                    out << YAML::Key << "Cascade Range 4" << YAML::Value << instance->camera.lock()->m_cascadeRanges[3];
                    out << YAML::EndMap;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeScriptClass(YAML::Emitter& out)
{
    if (!Script::s_scriptClasses.empty()) {
        out << YAML::Key << "Script Class";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& klass : Script::s_scriptClasses) {
            out << YAML::BeginMap;
            out << YAML::Key << "Script File Path" << YAML::Value << klass.second->m_filePath;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeScriptInstance(YAML::Emitter& out)
{
    if (!Script::s_scriptInstances.empty()) {
        out << YAML::Key << "Script Instance";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& instance : Script::s_scriptInstances) {
            out << YAML::BeginMap;
            out << YAML::Key << "Class Name" << YAML::Value << instance.second->m_scriptClass->m_fullName;
            out << YAML::Key << "Mesh Instance ID" << YAML::Value << instance.first;
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

    scene.m_sceneFolderPath = data["SceneFolderPath"].as<std::string>();
    scene.m_sceneFilePath = data["SceneFilePath"].as<std::string>();

    auto camera = data["Camera"];
    scene.m_mainCamera.m_pos = camera["Position"].as<glm::vec3>();
    scene.m_mainCamera.m_dir = camera["Dir"].as<glm::vec3>();
    scene.m_mainCamera.m_cascadeRanges[0] = camera["Cascade Range 1"].as<float>();
    scene.m_mainCamera.m_cascadeRanges[1] = camera["Cascade Range 2"].as<float>();
    scene.m_mainCamera.m_cascadeRanges[2] = camera["Cascade Range 3"].as<float>();
    scene.m_mainCamera.m_cascadeRanges[3] = camera["Cascade Range 4"].as<float>();
    scene.m_mainCamera.m_at = scene.m_mainCamera.m_pos + scene.m_mainCamera.m_dir;

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
            auto filePath = resource["FilePath"].as<std::string>();
            scene.AddResource(filePath);
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
                    scene.m_subCameras[meshInstance->UUID]->m_cascadeRanges[0] = subCamera["Cascade Range 1"].as<float>();
                    scene.m_subCameras[meshInstance->UUID]->m_cascadeRanges[1] = subCamera["Cascade Range 2"].as<float>();
                    scene.m_subCameras[meshInstance->UUID]->m_cascadeRanges[2] = subCamera["Cascade Range 3"].as<float>();
                    scene.m_subCameras[meshInstance->UUID]->m_cascadeRanges[3] = subCamera["Cascade Range 4"].as<float>();
                }
            }
        }
        scene.UpdateMeshBuffer();
    }

    auto scriptClasses = data["Script Class"];
    if (scriptClasses) {
        for (auto&& scriptClass : scriptClasses) {
            Script::LoadAssemblyClasses(scriptClass["Script File Path"].as<std::string>());
        }
    }

    auto scriptInstances = data["Script Instance"];
    if (scriptInstances) {
        for (auto&& scriptInstance : scriptInstances) {
            auto scriptClassName = scriptInstance["Class Name"].as<std::string>();
            auto meshInstanceID = scriptInstance["Mesh Instance ID"].as<uint64_t>();
            Script::s_scriptInstances.emplace(meshInstanceID, std::make_shared<ScriptInstance>(Script::s_scriptClasses[scriptClassName], meshInstanceID));
        }
    }
}
