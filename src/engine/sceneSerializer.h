#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#include "../common.h"
#include "../scene/scene.h"
#include "../struct.h"
#include <yaml-cpp/yaml.h>

class SceneSerializer
{
public:
    void Serialize(const Scene& scene, const std::string& filePath);
    void SerializeMeshes(YAML::Emitter& out, const std::vector<MeshModel>& meshes);
    void SerializeResources(YAML::Emitter& out, const std::vector<Resource>& resources);
    void Deserialize(Scene& scene, const std::string& filePath);
    void SerializeCamera(YAML::Emitter& out, const Camera& camera);
    void SerializePointLights(YAML::Emitter& out, const std::vector<PointLightUBO>& pointLights);
    void SerializeDirLight(YAML::Emitter& out, const Scene& scene);
};

#endif
