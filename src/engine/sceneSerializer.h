#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#include "../common.h"
#include "../scene/scene.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "script/script.h"

class SceneSerializer
{
public:
    void Serialize(const Scene& scene);
    void SerializeMesh(YAML::Emitter& out, const std::vector<std::shared_ptr<Mesh>>& meshes);
    void SerializeResource(YAML::Emitter& out, const std::vector<Resource>& resources);
    void Deserialize(Scene& scene, const std::string& filePath);
    void SerializeCamera(YAML::Emitter& out, const Camera& camera);
    void SerializePointLight(YAML::Emitter& out, const PointLight& pointLight);
    void SerializeDirLight(YAML::Emitter& out, const Scene& scene);
    void SerializeScriptClass(YAML::Emitter& out);
    void SerializeScriptInstance(YAML::Emitter& out);
};

#endif
