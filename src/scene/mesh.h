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
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"
#include "camera.h"

struct MeshPart;
struct MeshInstanceUBO;
struct MeshInstance;
struct PhysicsDebugUBO;
struct MaterialFilePath;

class Mesh : public MeshBase
{
    friend class UI;
    friend class Scene;
    friend class SceneSerializer;
    friend class Physics;

    void CreateSquare(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, const char* name = nullptr, const char* texturePath = nullptr);

    std::string m_filePath;
    std::string m_name;

    // id used for mouse picking
    int32_t m_meshColorID;
    std::vector<MeshInstanceUBO> m_meshInstanceUBOs;
    std::unique_ptr<vkn::Buffer> m_meshInstanceUBOBuffer;
    std::vector<std::unique_ptr<MeshInstance>> m_meshInstances;
    std::vector<MeshPart> m_meshParts;
    std::vector<MaterialFilePath> m_materials;

    void LoadModel(const std::string& filepath);
    void GetTransformation(const aiNode* node, aiMatrix4x4& transformation);
    void ProcessNode(const aiScene* scene, const aiNode* node);
    void ProcessLoadedMesh(const aiScene* scene, const aiMesh* mesh, glm::mat4& modelMat);
    void CreateBuffers(const vk::CommandBuffer& commandBuffer);
    void AddInstance(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddInstance(uint64_t UUID);
    void DeleteInstance(int32_t instanceColorID);

public:
    Mesh() = default;
    Mesh(int meshColorID) : m_meshColorID(meshColorID) {}
    Mesh(int meshColorID, const std::string& filePath, const std::string& fileName);
    const std::string& GetName() { return m_name; }
    int32_t GetMeshColorID() const { return m_meshColorID; }
    size_t GetInstanceCount() const { return m_meshInstances.size(); }
    size_t GetMaterialCount() const { return m_materials.size(); }
    const std::vector<std::unique_ptr<MeshInstance>>& GetInstances() const { return m_meshInstances; }
    const std::vector<MeshPart>& GetMeshParts() const { return m_meshParts; }
    btTriangleIndexVertexArray* GetBulletVertexArray() { return &m_bulletVertexArray; }
    void UpdateUBO(MeshInstance& instance);
    void UpdateColorID();
};

struct MeshPart
{
    int32_t bufferIndex;
    int32_t materialIndex;

    MeshPart(int32_t bufferIndex, int32_t materialIndex)
        : bufferIndex(bufferIndex), materialIndex(materialIndex) {}
};

struct MeshInstanceUBO
{
    glm::mat4 model = glm::mat4(1.0f);

    int32_t meshColorID = -1; // Color ID for mouse picking
    int32_t instanceColorID = -1;
    int32_t useAlbedoTexture = 1;
    int32_t useNormalTexture = 1;

    int32_t useMetallicTexture = 1;
    int32_t useRoughnessTexture = 1;
    float padding0[2];

    glm::vec3 albedo = glm::vec3(0.5f);
    float metallic = 0.0f;
    float roughness = 1.0f;
    float padding1[3];

    MeshInstanceUBO(int32_t meshColorID, int32_t instanceColorID, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
};

struct PhysicsDebugUBO
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

struct MeshInstance
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

    MeshInstance(uint64_t UUID, MeshInstanceUBO&& UBO);
    MeshInstance& operator=(const MeshInstance& other);
    void UpdateTransform();
    void UpdateMatrix();
};

struct MaterialFilePath
{
    std::string albedo;
    std::string normal;
    std::string metallic;
    std::string roughness;
};

#endif