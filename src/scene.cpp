#include "scene.h"

Scene::Scene()
{
    positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    m_triangleMesh = std::make_unique<Mesh>();
}