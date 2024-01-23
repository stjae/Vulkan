#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "API/swapchain.h"
#include "API/commands.h"
#include "viewport.h"

struct UboDataDynamic
{
    glm::mat4* model = nullptr;
    uint32_t alignment{};
};

class Scene
{
    Command command_;
    size_t meshCount_[3]{};
    std::unique_ptr<Buffer> matrixUniformBufferDynamic_;

    void CreateUniformBuffer();
    void UpdateBuffer();
    void UpdateMesh();

public:
    std::vector<Mesh> meshes;
    std::vector<std::array<std::string, 3>> resources;
    Camera camera;
    UboDataDynamic uboDataDynamic;

    Scene();
    void AddMesh(MeshType type);
    void AddMesh(MeshType type, const std::string& filePath);
    const char* GetMeshName(size_t index) { return meshes[index].name_.c_str(); }
    bool IsMeshSelected(size_t index) { return meshes[index].isSelected_; }
    void PrepareMeshes();
    void DeleteMesh(long index, MeshType type);
    void Update(uint32_t frameIndex, const std::vector<ViewportFrame>& viewportFrames);
    ~Scene();
};

#endif