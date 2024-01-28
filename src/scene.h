#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "API/swapchain.h"
#include "API/command.h"
#include "viewport.h"

struct UboDataDynamic
{
    glm::mat4* model = nullptr;
    uint32_t alignment{};
};

class Scene
{
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;
    std::unique_ptr<Buffer> matrixUniformBufferDynamic_;
    size_t meshCount_[3]{};

    void CreateUniformBuffer();
    void UpdateBuffer();
    void UpdateMesh();

public:
    std::vector<Mesh> meshes;
    std::vector<std::array<std::string, 3>> resources;
    Camera camera;
    UboDataDynamic uboDataDynamic;
    size_t meshSelected = -1;

    Scene();
    void AddMesh(MeshType type);
    void AddMesh(MeshType type, const std::string& filePath);
    [[nodiscard]] const char* GetMeshName(size_t index) const { return meshes[index].name_.c_str(); }
    [[nodiscard]] bool IsMeshSelected(size_t index) const { return meshes[index].isSelected_; }
    void PrepareMeshes();
    void DeleteMesh();
    void Update(uint32_t frameIndex, const std::vector<ViewportFrame>& viewportFrames);
    ~Scene();
};

#endif