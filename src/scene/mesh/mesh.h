#ifndef MESHPRIMITIVE_H
#define MESHPRIMITIVE_H

#include "meshBase.h"
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
    std::string name_;

    int32_t meshID_;
    std::unique_ptr<vkn::Buffer> meshInstanceBuffer_;
    std::vector<MeshInstanceUBO> meshInstanceUBOs_;
    std::vector<MeshPart> meshParts_;
    std::vector<MaterialFilePath> materials_;
    inline static vk::CommandBuffer commandBuffer_;

    void CreateBuffers();

public:
    std::unique_ptr<MeshPhysicsInfo> physicsInfo;
    std::unique_ptr<PhysicsDebugDrawer> physicsDebugDrawer;

    Mesh() = default;
    explicit Mesh(int meshID) : meshID_(meshID) {}
    void AddInstance(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    void AddPhysicsInfo(const MeshPhysicsInfo& physicsInfo);
    const std::string& GetName() { return name_; }
    int32_t GetMeshID() const { return meshID_; }
    size_t GetInstanceCount() const { return meshInstanceUBOs_.size(); }
    size_t GetMaterialCount() const { return materials_.size(); }
    const std::vector<MeshPart>& GetMeshParts() const { return meshParts_; }
};

#endif