#ifndef MESHPRIMITIVE_H
#define MESHPRIMITIVE_H

#include "meshBase.h"
#include "../../vulkan/sync.h"
#include "../../vulkan/command.h"
#include "tiny_obj_loader.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "../physicsDebugDrawer.h"

class Mesh : public MeshBase
{
    friend class Scene;
    friend class SceneSerializer;
    friend class Physics;

    void CreateSquare(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateCube(float scale = 1.0f, const char* texturePath = nullptr);
    void CreateSphere(float scale = 1.0f, const char* name = nullptr, const char* texturePath = nullptr);

protected:
    std::string m_name;

    int32_t m_meshID;
    std::unique_ptr<vkn::Buffer> m_meshInstanceBuffer;
    std::vector<MeshInstanceUBO> m_meshInstanceUBOs;
    std::vector<MeshPart> m_meshParts;
    std::vector<MaterialFilePath> m_materials;

    void CreateBuffers(const vk::CommandBuffer& commandBuffer);

public:
    std::unique_ptr<MeshPhysicsInfo> m_physicsInfo;
    std::unique_ptr<PhysicsDebugDrawer> m_physicsDebugDrawer;

    Mesh() = default;
    explicit Mesh(int meshID) : m_meshID(meshID) {}
    void AddInstance(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddPhysicsInfo(const MeshPhysicsInfo& physicsInfo);
    const std::string& GetName() { return m_name; }
    int32_t GetMeshID() const { return m_meshID; }
    size_t GetInstanceCount() const { return m_meshInstanceUBOs.size(); }
    size_t GetMaterialCount() const { return m_materials.size(); }
    const std::vector<MeshPart>& GetMeshParts() const { return m_meshParts; }
};

#endif