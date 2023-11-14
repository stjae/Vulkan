#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "mesh.h"
#include "light.h"
#include <filesystem>

class Scene
{
public:
    Scene();
    void CreateResource(const Device& device);

    std::vector<std::unique_ptr<Mesh>> meshes;
    std::unique_ptr<Light> pointLight;
};

#endif