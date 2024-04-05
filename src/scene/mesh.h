#ifndef MESH_H
#define MESH_H

#include "meshBase.h"
#include "../vulkan/command.h"
#include "tiny_obj_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

enum MESHTYPE {
    SQUARE,
    CUBE,
    SPHERE,
};

struct MeshPart
{
    int32_t bufferIndex;
    int32_t materialID;

    MeshPart(int32_t bufferIndex, int32_t materialID) : bufferIndex(bufferIndex), materialID(materialID) {}
};

struct MeshInstance
{
    glm::mat4 model;
    glm::mat4 invTranspose;
    int32_t meshID;
    int32_t textureID;
    int32_t instanceID;
    int32_t useTexture;
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float padding[3];

    MeshInstance(int32_t meshID, int32_t instanceID, glm::vec3& pos, glm::vec3& scale) : model(1.0f), invTranspose(1.0f), meshID(meshID), textureID(0), useTexture(false), instanceID(instanceID), albedo({ 0.5, 0.5, 0.5 }), metallic(0.0f), roughness(1.0f)
    {
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
        invTranspose = model;
        invTranspose[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        invTranspose = glm::transpose(glm::inverse(invTranspose));
    }
};

struct Material
{
    std::string albedo;
    std::string normal;
    std::string metallic;
    std::string roughness;
};

class Mesh : public MeshBase
{
    friend class Scene;

    std::string name_;

    int32_t meshID_;
    int32_t instanceID_;
    std::unique_ptr<vkn::Buffer> meshInstanceBuffer_;
    std::vector<MeshInstance> meshInstances_;
    std::vector<MeshPart> meshParts_;
    std::vector<Material> materials_;

    void CreateSquare(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, const char* name = nullptr, const char* texturePath = nullptr);
    void LoadModel(const std::string& modelPath, const char* texturePath = nullptr, glm::vec3 color = glm::vec3(0.5f));
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessLoadedMesh(aiMesh* mesh, glm::mat4& modelMat);
    void CreateBuffers();

    inline static vk::CommandBuffer commandBuffer_;

public:
    explicit Mesh(MESHTYPE meshType);
    Mesh(uint32_t meshID, const std::string& filePath);
    const std::string& GetName() { return name_; }
    int32_t GetMeshID() const { return meshID_; }
    size_t GetInstanceCount() const { return meshInstances_.size(); }
    size_t GetMaterialCount() const { return materials_.size(); }
    const std::vector<MeshPart>& GetMeshParts() const { return meshParts_; }
};

#endif