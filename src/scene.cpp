#include "scene.h"

Scene::Scene()
{
    positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    // std::unique_ptr<Mesh> square = std::make_unique<Mesh>();
    // square->CreateSquare();

    // meshes.push_back(std::move(square));

    std::unique_ptr<Mesh> cube = std::make_unique<Mesh>();
    cube->CreateCube();

    meshes.push_back(std::move(cube));
}

void Scene::CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer(vkPhysicalDevice, vkDevice);
        mesh->CreateVertexBuffer(vkPhysicalDevice, vkDevice);
    }
}
