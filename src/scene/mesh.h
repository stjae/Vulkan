#ifndef MESH_H
#define MESH_H

#include "meshData.h"
#include "../vulkan/command.h"

struct InstanceData
{
    glm::mat4 model;
    glm::mat4 invTranspose;
    int32_t meshID;
    int32_t textureID;
    int useTexture;
    int32_t instanceID;

    explicit InstanceData(int32_t meshID, int32_t instanceID) : model(1.0f), invTranspose(1.0f), meshID(meshID), textureID(0), useTexture(0), instanceID(instanceID) {}
};

class Mesh : public MeshData
{
    friend class Engine;
    friend class Scene;
    friend class UI;

    std::string name_;
    glm::vec3 position_;
    glm::vec3 rotation_;

    int32_t meshID;
    int32_t instanceID;
    std::vector<InstanceData> instanceData_;

    void CreateSquare(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, glm::vec3 color = glm::vec3(0.5f), const char* name = nullptr, const char* texturePath = nullptr);
    void LoadModel(const std::string& modelPath, const char* texturePath = nullptr, glm::vec3 color = glm::vec3(0.5f));
    void CreateBuffers();

    inline static vk::CommandBuffer commandBuffer_;

public:
    explicit Mesh(MESHTYPE meshType);
    Mesh(uint32_t meshID, const std::string& filePath);
    const std::string& GetName() { return name_; }
    size_t GetInstanceCount() { return instanceData_.size(); }
};

#endif