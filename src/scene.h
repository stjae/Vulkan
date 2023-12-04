#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "camera.h"
#include "mesh.h"
#include "API/swapchain.h"
#include "API/commands.h"

class Scene
{
public:
    Scene();
    void Prepare(const Device& device);
    void CreateResource(const Device& device);
    void Update(uint32_t index, const Swapchain& swapchain, const vk::Device& vkDevice);

    Camera camera_;
    std::vector<std::unique_ptr<Mesh>> meshes_;
};

#endif