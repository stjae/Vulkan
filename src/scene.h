#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "mesh.h"

class Scene
{
public:
    Scene();
    void CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::vector<glm::vec3> positions;
};

#endif