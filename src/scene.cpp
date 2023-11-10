#include "scene.h"

Scene::Scene()
{
    std::unique_ptr<Mesh> cube = std::make_unique<Mesh>();
    std::unique_ptr<Mesh> square = std::make_unique<Mesh>();
    cube->CreateCube();
    square->CreateSquare();

    meshes.push_back(std::move(cube));
    meshes.push_back(std::move(square));

    pointLight = std::make_unique<Light>();
}

void Scene::CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer(vkPhysicalDevice, vkDevice);
        mesh->CreateVertexBuffer(vkPhysicalDevice, vkDevice);
    }
}
