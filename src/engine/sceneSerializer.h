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
    void SerializeMesh(YAML::Emitter& out, const Scene& scene);
    void SerializeResource(YAML::Emitter& out, const Scene& scene);
    void SerializeCamera(YAML::Emitter& out, const Scene& scene);
    void SerializePointLight(YAML::Emitter& out, const Scene& scene);
    void SerializeDirLight(YAML::Emitter& out, const Scene& scene);
    void SerializeScriptClass(YAML::Emitter& out, const Scene& scene);
    void SerializeScriptInstance(YAML::Emitter& out);
    void Deserialize(Scene& scene, const std::string& filePath);
};

#endif
