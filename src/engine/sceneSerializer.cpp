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

    if (!scene.m_saveFilePath.empty()) {
        out << YAML::Key << "SaveFilePath" << YAML::Value << filePath;
    }
    if (!scene.m_hdriFilePath.empty()) {
        out << YAML::Key << "HDRIFilePath" << YAML::Value << scene.m_hdriFilePath;
    }
    out << YAML::Key << "IBLExposure" << YAML::Value << scene.m_iblExposure;
    SerializeDirLight(out, scene);
    SerializePointLights(out, scene.m_pointLights);
    SerializeCamera(out, scene.m_camera);
    SerializeResources(out, scene.m_resources);
    SerializeMeshes(out, scene.m_meshes);

    out << YAML::EndMap;

    std::ofstream fout(filePath);
    fout << out.c_str();
}

void SceneSerializer::SerializeDirLight(YAML::Emitter& out, const Scene& scene)
{
    out << YAML::Key << "DirectionalLight" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "NearPlane" << YAML::Value << scene.m_dirLightNearPlane;
    out << YAML::Key << "FarPlane" << YAML::Value << scene.m_dirLightFarPlane;
    out << YAML::Key << "Distance" << YAML::Value << scene.m_dirLightDistance;
    out << YAML::Key << "Rotation" << YAML::Value << scene.m_dirLightRot;
    out << YAML::Key << "Position" << YAML::Value << scene.m_dirLightPos;
    out << YAML::Key << "Color" << YAML::Value << scene.m_dirLightUBO.color;
    out << YAML::Key << "Intensity" << YAML::Value << scene.m_dirLightUBO.intensity;
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
    out << YAML::Key << "Position" << YAML::Value << camera.m_pos;
    out << YAML::Key << "Dir" << YAML::Value << camera.m_dir;
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
            out << YAML::Key << "ID" << YAML::Value << mesh.m_meshID;
            if (mesh.m_physicsInfo) {
                out << YAML::Key << "RigidBody" << YAML::Value;
                out << YAML::BeginMap;
                out << YAML::Key << "Type" << YAML::Value << (int)mesh.m_physicsInfo->rigidBodyType;
                out << YAML::Key << "Shape" << YAML::Value << (int)mesh.m_physicsInfo->colliderShape;
                out << YAML::EndMap;
            }
            out << YAML::Key << "Instances";
            out << YAML::Value << YAML::BeginSeq;
            for (auto& instanceUBO : mesh.m_meshInstanceUBOs) {
                out << YAML::BeginMap;
                out << YAML::Key << "ID" << YAML::Value << instanceUBO.instanceID;
                out << YAML::Key << "MeshID" << YAML::Value << instanceUBO.meshID;
                out << YAML::Key << "Transform" << YAML::Value << instanceUBO.model;
                out << YAML::Key << "InvTranspose" << YAML::Value << instanceUBO.invTranspose;
                out << YAML::Key << "Albedo" << YAML::Value << instanceUBO.albedo;
                out << YAML::Key << "Metallic" << YAML::Value << instanceUBO.metallic;
                out << YAML::Key << "Roughness" << YAML::Value << instanceUBO.roughness;
                out << YAML::Key << "RigidBodySize" << YAML::Value << instanceUBO.physicsInfo->size;
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
        scene.m_saveFilePath = data["SaveFilePath"].as<std::string>();
    }

    auto camera = data["Camera"];
    scene.m_camera.m_pos = camera["Position"].as<glm::vec3>();
    scene.m_camera.m_dir = camera["Dir"].as<glm::vec3>();
    scene.m_camera.m_at = scene.m_camera.m_pos + scene.m_camera.m_dir;

    auto hdriFilePath = data["HDRIFilePath"];
    if (hdriFilePath) {
        scene.AddEnvironmentMap(hdriFilePath.as<std::string>());
        scene.m_hdriFilePath = hdriFilePath.as<std::string>();
    }

    auto iblExposure = data["IBLExposure"];
    if (iblExposure) {
        scene.m_iblExposure = iblExposure.as<float>();
    }

    auto dirLight = data["DirectionalLight"];
    scene.m_dirLightNearPlane = dirLight["NearPlane"].as<float>();
    scene.m_dirLightFarPlane = dirLight["FarPlane"].as<float>();
    scene.m_dirLightDistance = dirLight["Distance"].as<float>();
    scene.m_dirLightRot = dirLight["Rotation"].as<glm::mat4>();
    scene.m_dirLightPos = dirLight["Position"].as<glm::vec3>();
    scene.m_dirLightUBO.color = dirLight["Color"].as<glm::vec3>();
    scene.m_dirLightUBO.intensity = dirLight["Intensity"].as<float>();

    auto pointLights = data["PointLights"];
    if (pointLights) {
        for (auto pointLight : pointLights) {
            scene.AddLight();
            scene.m_pointLights.back().model = pointLight["Transform"].as<glm::mat4>();
            scene.m_pointLights.back().pos = pointLight["Position"].as<glm::vec3>();
            scene.m_pointLights.back().color = pointLight["Color"].as<glm::vec3>();
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
            auto rigidBody = mesh["RigidBody"];
            if (rigidBody) {
                MeshPhysicsInfo pInfo;
                pInfo.rigidBodyType = (eRigidBodyType)rigidBody["Type"].as<int>();
                pInfo.colliderShape = (eColliderShape)rigidBody["Shape"].as<int>();
                scene.m_meshes[mesh["ID"].as<int>()].AddPhysicsInfo(pInfo);
            }
            auto instances = mesh["Instances"];
            for (auto instance : instances) {
                scene.AddMeshInstance(mesh["ID"].as<int>());
                auto& meshInstance = scene.m_meshes[mesh["ID"].as<int>()].m_meshInstanceUBOs.back();
                meshInstance.model = instance["Transform"].as<glm::mat4>();
                meshInstance.invTranspose = instance["InvTranspose"].as<glm::mat4>();
                meshInstance.albedo = instance["Albedo"].as<glm::vec3>();
                meshInstance.metallic = instance["Metallic"].as<float>();
                meshInstance.roughness = instance["Roughness"].as<float>();
                meshInstance.physicsInfo->size = instance["RigidBodySize"].as<glm::vec3>();
            }
        }
    }
}
