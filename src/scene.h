#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "camera.h"
#include "mesh.h"
#include "API/swapchain.h"
#include "API/commands.h"

struct MeshCount
{
    int square = 0;
    int cube = 0;
    int model = 0;
};

struct UboDataDynamic
{
    glm::mat4* model = nullptr;
    size_t alignment;
};

class Scene
{
    friend class MyImGui;
    friend class Command;

    MeshCount meshCount_;
    UboDataDynamic uboDataDynamic_;
    std::unique_ptr<Buffer> matrixUniformBufferDynamic_;

    std::unique_ptr<Camera> camera_;

    void CreateUniformBuffers();
    void UpdateBuffer();

public:
    std::vector<std::shared_ptr<Mesh>> meshes;

    Scene();
    void PrepareMeshes();
    void UpdateMesh();
    void Update(uint32_t index);
    ~Scene();
};

#endif