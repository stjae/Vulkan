#ifndef MESH_H
#define MESH_H

#include "meshData.h"
#include "../vulkan/command.h"
#include "tiny_obj_loader.h"

struct MeshPart
{
    int32_t materialID;
    int32_t indexCount;
    int32_t indexBase;

    MeshPart(int32_t mID, int32_t idxCount, int32_t idxBase) : materialID(mID), indexCount(idxCount), indexBase(idxBase) {}
};

struct MeshInstance
{
    glm::mat4 model;
    glm::mat4 invTranspose;
    int32_t meshID;
    int32_t textureID;
    bool useTexture;
    int32_t instanceID;

    MeshInstance(int32_t meshID, int32_t instanceID, glm::vec3& pos, glm::vec3& scale) : model(1.0f), invTranspose(1.0f), meshID(meshID), textureID(0), useTexture(false), instanceID(instanceID)
    {
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
        invTranspose = model;
        invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        invTranspose = glm::transpose(glm::inverse(invTranspose));
    }
};

class Mesh : public MeshBase
{
    friend class Scene;

    std::string name_;

    int32_t meshID_;
    int32_t instanceID_;
    std::unique_ptr<Buffer> meshInstanceBuffer_;
    std::vector<MeshInstance> meshInstances_;
    std::vector<MeshPart> meshParts_;
    std::vector<tinyobj::material_t> materials_;

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
    int32_t GetMeshID() const { return meshID_; }
    size_t GetInstanceCount() const { return meshInstances_.size(); }
    const std::vector<MeshPart>& GetMeshParts() const { return meshParts_; }
};

#endif