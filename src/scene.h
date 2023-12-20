#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "camera.h"
#include "mesh.h"
#include "API/swapchain.h"
#include "API/commands.h"

struct UboDataDynamic
{
    glm::mat4* model = nullptr;
};

class Scene
{
    friend class MyImGui;

    UboDataDynamic uboDataDynamic;
    std::unique_ptr<Buffer> matrixUniformBufferDynamic;

    void SetDynamicBufferAlignmentSize();
    void CreateUniformBuffers();
    void UpdateBuffer();

public:
    std::unique_ptr<Camera> camera;
    std::vector<std::shared_ptr<Mesh>> meshes;

    Scene();
    void PrepareScene();
    void PrepareMeshes();
    void Update(uint32_t index);
    ~Scene();
};

#endif