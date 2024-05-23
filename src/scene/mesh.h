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
#include "../struct.h"

class Mesh : public MeshBase
{
    friend class Scene;

    void CreateSquare(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, const char* name = nullptr, const char* texturePath = nullptr);

    std::string m_filepath;
    std::string m_name;

    // id used for mouse picking
    int32_t m_meshColorID;
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

public:
    explicit Mesh(int meshColorID) : m_meshColorID(meshColorID) {}
    Mesh(int meshColorID, const std::string& filePath);
    const std::string& GetName() { return m_name; }
    int32_t GetMeshColorID() const { return m_meshColorID; }
    size_t GetInstanceCount() const { return m_meshInstances.size(); }
    size_t GetMaterialCount() const { return m_materials.size(); }
    const std::vector<std::unique_ptr<MeshInstance>>& GetInstances() const { return m_meshInstances; }
    const std::vector<MeshPart>& GetMeshParts() const { return m_meshParts; }
    btTriangleIndexVertexArray* GetBulletVertexArray() { return &m_bulletVertexArray; }
};

#endif