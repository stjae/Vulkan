#include "scene.h"

Scene::Scene()
{
    positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

    meshes.emplace_back(new Mesh());
}

void Scene::CreateResource(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer(vkPhysicalDevice, vkDevice);
        mesh->CreateVertexBuffer(vkPhysicalDevice, vkDevice);
    }
}
