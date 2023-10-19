#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "mesh.h"

class Scene
{
public:
    Scene();

    std::vector<glm::vec3> positions;
    std::unique_ptr<Mesh> m_triangleMesh;
};

#endif