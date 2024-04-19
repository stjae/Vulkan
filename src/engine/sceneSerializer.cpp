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

void SceneSerializer::Serialize(const Scene& scene, const std::string& filePath)
{
    YAML::Emitter out;

    out << YAML::BeginMap;

    if (!scene.saveFilePath_.empty()) {
        out << YAML::Key << "SaveFilePath" << YAML::Value << filePath;
    }
    if (!scene.hdriFilePath_.empty()) {
        out << YAML::Key << "HDRIFilePath" << YAML::Value << scene.hdriFilePath_;
    }
    out << YAML::Key << "IBLExposure" << YAML::Value << scene.iblExposure_;
    SerializeLights(out, scene.pointLights_);
    SerializeCamera(out, scene.camera_);
    SerializeResources(out, scene.resources_);
    SerializeMeshes(out, scene.meshes_);

    out << YAML::EndMap;

    std::ofstream fout(filePath);
    fout << out.c_str();
}

void SceneSerializer::SerializeLights(YAML::Emitter& out, const std::vector<LightData>& pointLights)
{
    if (!pointLights.empty()) {
        out << YAML::Key << "PointLights";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& pointLight : pointLights) {
            out << YAML::BeginMap;
            out << YAML::Key << "Transform" << YAML::Value << pointLight.model;
            out << YAML::Key << "Position" << YAML::Value << pointLight.pos;
            out << YAML::Key << "Color" << YAML::Value << pointLight.color;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeCamera(YAML::Emitter& out, const Camera& camera)
{
    out << YAML::Key << "Camera" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Position" << YAML::Value << camera.pos_;
    out << YAML::Key << "At" << YAML::Value << camera.at_;
    out << YAML::EndMap;
}

void SceneSerializer::SerializeResources(YAML::Emitter& out, const std::vector<Resource>& resources)
{
    if (!resources.empty()) {
        out << YAML::Key << "Resources";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& resource : resources) {
            out << YAML::BeginMap;
            out << YAML::Key << "FilePath" << YAML::Value << resource.filePath;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::SerializeMeshes(YAML::Emitter& out, const std::vector<MeshModel>& meshes)
{
    if (!meshes.empty()) {
        out << YAML::Key << "Meshes";
        out << YAML::Value << YAML::BeginSeq;
        for (auto& mesh : meshes) {
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << mesh.meshID_;
            out << YAML::Key << "Instances";
            out << YAML::Value << YAML::BeginSeq;
            if (!mesh.meshInstances_.empty()) {
                for (auto& instance : mesh.meshInstances_) {
                    out << YAML::BeginMap;
                    out << YAML::Key << "ID" << YAML::Value << instance.instanceID;
                    out << YAML::Key << "MeshID" << YAML::Value << instance.meshID;
                    out << YAML::Key << "Transform" << YAML::Value << instance.model;
                    out << YAML::Key << "InvTranspose" << YAML::Value << instance.invTranspose;
                    out << YAML::Key << "Albedo" << YAML::Value << instance.albedo;
                    out << YAML::Key << "Metallic" << YAML::Value << instance.metallic;
                    out << YAML::Key << "Roughness" << YAML::Value << instance.roughness;
                    out << YAML::EndMap;
                }
            }
            out << YAML::EndSeq;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    }
}

void SceneSerializer::Deserialize(Scene& scene, const std::string& filePath)
{
    scene.InitScene();

    std::ifstream stream(filePath);
    std::stringstream strStream;
    strStream << stream.rdbuf();

    YAML::Node data = YAML::Load(strStream.str());

    auto saveFilePath = data["SaveFilePath"];
    if (saveFilePath) {
        scene.saveFilePath_ = data["SaveFilePath"].as<std::string>();
    }

    auto camera = data["Camera"];
    scene.camera_.pos_ = camera["Position"].as<glm::vec3>();
    scene.camera_.at_ = camera["At"].as<glm::vec3>();

    auto hdriFilePath = data["HDRIFilePath"];
    if (hdriFilePath) {
        scene.AddEnvironmentMap(hdriFilePath.as<std::string>());
        scene.hdriFilePath_ = hdriFilePath.as<std::string>();
    }

    auto iblExposure = data["IBLExposure"];
    if (iblExposure) {
        scene.iblExposure_ = iblExposure.as<float>();
    }

    auto pointLights = data["PointLights"];
    if (pointLights) {
        for (auto pointLight : pointLights) {
            scene.AddLight();
            scene.pointLights_.back().model = pointLight["Transform"].as<glm::mat4>();
            scene.pointLights_.back().pos = pointLight["Position"].as<glm::vec3>();
            scene.pointLights_.back().color = pointLight["Color"].as<glm::vec3>();
        }
    }

    auto resources = data["Resources"];
    if (resources) {
        for (auto resource : resources) {
            auto filePath = resource["FilePath"].as<std::string>();
            scene.AddResource(filePath);
        }
    }

    auto meshes = data["Meshes"];
    if (meshes) {
        for (auto mesh : meshes) {
            auto instances = mesh["Instances"];
            for (auto instance : instances) {
                scene.AddMeshInstance(mesh["ID"].as<int>());
                auto& meshInstance = scene.meshes_[mesh["ID"].as<int>()].meshInstances_.back();
                meshInstance.model = instance["Transform"].as<glm::mat4>();
                meshInstance.invTranspose = instance["InvTranspose"].as<glm::mat4>();
                meshInstance.albedo = instance["Albedo"].as<glm::vec3>();
                meshInstance.metallic = instance["Metallic"].as<float>();
                meshInstance.roughness = instance["Roughness"].as<float>();
            }
        }
    }
}