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
    float dummy[2];
};

class Scene
{
    friend class MyImGui;

    UboDataDynamic uboDataDynamic;
    std::unique_ptr<Buffer> matrixUniformBufferDynamic;

    void PrepareUniformBuffers();
    void CreateResource();
    void UpdateBuffer();

public:
    std::unique_ptr<Camera> camera;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<const char*> meshNames;

    Scene();
    void Prepare();
    void Update(uint32_t index);
    ~Scene();
};

#endif