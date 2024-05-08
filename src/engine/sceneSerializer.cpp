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
    SerializeDirLight(out, scene);
    SerializePointLights(out, scene.pointLights_);
    SerializeCamera(out, scene.camera_);
    SerializeResources(out, scene.resources_);
    SerializeMeshes(out, scene.meshes_);

    out << YAML::EndMap;

    std::ofstream fout(filePath);
    fout << out.c_str();
}

void SceneSerializer::SerializeDirLight(YAML::Emitter& out, const Scene& scene)
{
    out << YAML::Key << "DirectionalLight" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "NearPlane" << YAML::Value << scene.dirLightNearPlane_;
    out << YAML::Key << "FarPlane" << YAML::Value << scene.dirLightFarPlane_;
    out << YAML::Key << "Distance" << YAML::Value << scene.dirLightDistance_;
    out << YAML::Key << "Rotation" << YAML::Value << scene.dirLightRot_;
    out << YAML::Key << "Position" << YAML::Value << scene.dirLightPos_;
    out << YAML::Key << "Color" << YAML::Value << scene.dirLightUBO_.color;
    out << YAML::Key << "Intensity" << YAML::Value << scene.dirLightUBO_.intensity;
    out << YAML::EndMap;
}

void SceneSerializer::SerializePointLights(YAML::Emitter& out, const std::vector<PointLightUBO>& pointLights)
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
    out << YAML::Key << "Dir" << YAML::Value << camera.dir_;
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
            for (auto& instanceUBO : mesh.meshInstanceUBOs_) {
                out << YAML::BeginMap;
                out << YAML::Key << "ID" << YAML::Value << instanceUBO.instanceID;
                out << YAML::Key << "MeshID" << YAML::Value << instanceUBO.meshID;
                out << YAML::Key << "Transform" << YAML::Value << instanceUBO.model;
                out << YAML::Key << "InvTranspose" << YAML::Value << instanceUBO.invTranspose;
                out << YAML::Key << "Albedo" << YAML::Value << instanceUBO.albedo;
                out << YAML::Key << "Metallic" << YAML::Value << instanceUBO.metallic;
                out << YAML::Key << "Roughness" << YAML::Value << instanceUBO.roughness;
                if (instanceUBO.pInfo) {
                    out << YAML::Key << "RigidBody";
                    out << YAML::BeginMap;
                    out << YAML::Key << "Type" << YAML::Value << (int)instanceUBO.pInfo->rigidBodyType;
                    out << YAML::Key << "Shape" << YAML::Value << (int)instanceUBO.pInfo->rigidBodyShape;
                    out << YAML::Key << "Matrix" << YAML::Value << instanceUBO.pInfo->matrix;
                    out << YAML::Key << "Scale" << YAML::Value << instanceUBO.pInfo->scale;
                    out << YAML::Key << "Size" << YAML::Value << instanceUBO.pInfo->size;
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
    scene.camera_.dir_ = camera["Dir"].as<glm::vec3>();
    scene.camera_.at_ = scene.camera_.pos_ + scene.camera_.dir_;

    auto hdriFilePath = data["HDRIFilePath"];
    if (hdriFilePath) {
        scene.AddEnvironmentMap(hdriFilePath.as<std::string>());
        scene.hdriFilePath_ = hdriFilePath.as<std::string>();
    }

    auto iblExposure = data["IBLExposure"];
    if (iblExposure) {
        scene.iblExposure_ = iblExposure.as<float>();
    }

    auto dirLight = data["DirectionalLight"];
    scene.dirLightNearPlane_ = dirLight["NearPlane"].as<float>();
    scene.dirLightFarPlane_ = dirLight["FarPlane"].as<float>();
    scene.dirLightDistance_ = dirLight["Distance"].as<float>();
    scene.dirLightRot_ = dirLight["Rotation"].as<glm::mat4>();
    scene.dirLightPos_ = dirLight["Position"].as<glm::vec3>();
    scene.dirLightUBO_.color = dirLight["Color"].as<glm::vec3>();
    scene.dirLightUBO_.intensity = dirLight["Intensity"].as<float>();

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
                auto& meshInstance = scene.meshes_[mesh["ID"].as<int>()].meshInstanceUBOs_.back();
                meshInstance.model = instance["Transform"].as<glm::mat4>();
                meshInstance.invTranspose = instance["InvTranspose"].as<glm::mat4>();
                meshInstance.albedo = instance["Albedo"].as<glm::vec3>();
                meshInstance.metallic = instance["Metallic"].as<float>();
                meshInstance.roughness = instance["Roughness"].as<float>();
                auto rigidBody = instance["RigidBody"];
                if (rigidBody) {
                    MeshInstancePhysicsInfo pInfo;
                    pInfo.matrix = rigidBody["Matrix"].as<glm::mat4>();
                    pInfo.rigidBodyType = (eRigidBodyType)rigidBody["Type"].as<int>();
                    pInfo.rigidBodyShape = (eRigidBodyShape)rigidBody["Shape"].as<int>();
                    pInfo.scale = rigidBody["Scale"].as<glm::vec3>();
                    pInfo.size = rigidBody["Size"].as<glm::vec3>();
                    scene.physics_.AddRigidBody(meshInstance, pInfo);
                }
            }
        }
    }
}
