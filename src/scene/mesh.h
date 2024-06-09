#ifndef MESHPRIMITIVE_H
#define MESHPRIMITIVE_H

#include "meshBase.h"
#include "../vulkan/sync.h"
#include "../vulkan/command.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "physicsDebugDrawer.h"
#include "id.h"
#include "camera.h"

typedef struct MeshPart_ MeshPart;
typedef struct MeshInstanceUBO_ MeshInstanceUBO;
typedef struct MeshInstance_ MeshInstance;
typedef struct PhysicsDebugUBO_ PhysicsDebugUBO;
typedef struct MaterialFilePath_ MaterialFilePath;

class Mesh : public MeshBase
{
    friend class Scene;
    friend class Physics;

    void CreateSquare(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, const char* name = nullptr, const char* texturePath = nullptr);

    std::string m_filepath;
    std::string m_name;

    // id used for mouse picking
    int32_t m_meshColorID;
    std::vector<MeshInstanceUBO> m_meshInstanceUBOs;
    std::unique_ptr<vkn::Buffer> m_meshInstanceUBOBuffer;
    std::vector<std::unique_ptr<MeshInstance>> m_meshInstances;
    std::vector<MeshPart> m_meshParts;
    std::vector<MaterialFilePath> m_materials;

    void LoadModel(const std::string& filepath);
    void ProcessNode(aiNode* node, const aiScene* scene);
    void ProcessLoadedMesh(aiMesh* mesh, glm::mat4& modelMat);
    void CreateBuffers(const vk::CommandBuffer& commandBuffer);
    void AddInstance(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddInstance(uint64_t UUID);
    void DeleteInstance(int32_t instanceColorID);

public:
    Mesh() = default;
    Mesh(int meshColorID) : m_meshColorID(meshColorID) {}
    Mesh(int meshColorID, const std::string& filePath);
    const std::string& GetName() { return m_name; }
    int32_t GetMeshColorID() const { return m_meshColorID; }
    size_t GetInstanceCount() const { return m_meshInstances.size(); }
    size_t GetMaterialCount() const { return m_materials.size(); }
    const std::vector<std::unique_ptr<MeshInstance>>& GetInstances() const { return m_meshInstances; }
    const std::vector<MeshPart>& GetMeshParts() const { return m_meshParts; }
    btTriangleIndexVertexArray* GetBulletVertexArray() { return &m_bulletVertexArray; }
    void UpdateUBO(MeshInstance& instance);
};

typedef struct MeshPart_
{
    int32_t bufferIndex;
    int32_t materialID;

    MeshPart_(int32_t bufferIndex, int32_t materialID) : bufferIndex(bufferIndex), materialID(materialID) {}
} MeshPart_;

typedef struct MeshInstanceUBO_
{
    glm::mat4 model = glm::mat4(1.0f);
    // Color ID for mouse picking
    int32_t meshColorID;
    int32_t textureID = 0;
    int32_t instanceColorID;
    int32_t useTexture = true;
    glm::vec3 albedo = glm::vec3(0.5f);
    float metallic = 0.0f;
    float roughness = 1.0f;
    float padding[3];

    MeshInstanceUBO_(int32_t meshColorID, int32_t instanceColorID, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)) : meshColorID(meshColorID), instanceColorID(instanceColorID)
    {
        model = glm::translate(model, pos);
        model = glm::scale(model, scale);
    }
} MeshInstanceUBO_;

typedef struct PhysicsDebugUBO_
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    // flag for shader
    int32_t havePhysicsInfo = 0;
} PhysicsDebugUBO_;

typedef struct MeshInstance_
{
    const uint64_t UUID;
    glm::vec3 translation = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    MeshInstanceUBO UBO;
    std::unique_ptr<PhysicsDebugDrawer> physicsDebugDrawer;
    std::unique_ptr<PhysicsInfo> physicsInfo;
    PhysicsDebugUBO physicsDebugUBO;
    std::unique_ptr<vkn::Buffer> physicsDebugUBOBuffer;
    std::unique_ptr<SubCamera> camera;

    MeshInstance_(uint64_t UUID, MeshInstanceUBO&& UBO)
        : UUID(UUID), UBO(UBO)
    {
        vkn::BufferInfo bufferInfo = { sizeof(PhysicsDebugUBO), vk::WholeSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        physicsDebugUBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
    }
    MeshInstance_& operator=(const MeshInstance& other)
    {
        this->translation = other.translation;
        this->rotation = other.rotation;
        this->scale = other.scale;
        this->UBO = other.UBO;
        this->physicsDebugUBO.model = UBO.model;
        return *this;
    }
    // update translation, rotation, scale based on model matrix
    void UpdateTransform()
    {
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(UBO.model), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
    }
    // update matrix based on translation, rotation, scale
    void UpdateMatrix()
    {
        ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale), glm::value_ptr(UBO.model));
    }
} MeshInstance_;

typedef struct MaterialFilePath_
{
    std::string albedo;
    std::string normal;
    std::string metallic;
    std::string roughness;
} MaterialFilePath_;

#endif