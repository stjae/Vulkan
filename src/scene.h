#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "API/swapchain.h"
#include "API/commands.h"
#include "viewport.h"

struct MeshCount
{
    int square = 0;
    int cube = 0;
    int model = 0;
};

struct UboDataDynamic
{
    glm::mat4* model = nullptr;
    uint32_t alignment;
};

class Scene
{
    friend class Engine;
    friend class MyImGui;

    Command command_;
    std::vector<std::array<std::string, 2>> resources_;
    MeshCount meshCount_;
    UboDataDynamic uboDataDynamic_;
    std::unique_ptr<Buffer> matrixUniformBufferDynamic_;
    Camera camera_;

    void CreateUniformBuffers();
    void UpdateBuffer();

public:
    std::vector<Mesh> meshes;

    Scene();
    void PrepareMeshes();
    void UpdateMesh();
    void Update(uint32_t frameIndex, const std::vector<ViewportFrame>& viewportFrames);
    ~Scene();
};

#endif