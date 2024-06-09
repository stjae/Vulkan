#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#include "../common.h"
#include "../scene/scene.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

class SceneSerializer
{
public:
    void Serialize(const Scene& scene);
    void SerializeMeshes(YAML::Emitter& out, const std::vector<std::shared_ptr<Mesh>>& meshes);
    void SerializeResources(YAML::Emitter& out, const std::vector<Resource>& resources);
    void Deserialize(Scene& scene, const std::string& filePath, const vk::CommandBuffer& commandBuffer);
    void SerializeCamera(YAML::Emitter& out, const Camera& camera);
    void SerializePointLight(YAML::Emitter& out, const PointLight& pointLight);
    void SerializeDirLight(YAML::Emitter& out, const Scene& scene);
};

#endif
