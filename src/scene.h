#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "mesh.h"

class Scene
{
public:
    Scene();
    void CreateVertexBuffer(std::shared_ptr<Mesh> mesh);
    void CreateIndexBuffer(std::shared_ptr<Mesh> mesh);

    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::vector<glm::vec3> m_positions;
};

#endif