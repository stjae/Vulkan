#include "scene.h"

Scene::Scene()
{
    meshes.emplace_back(std::make_unique<Mesh>());
    meshes.back()->CreateCube(nullptr);
    meshes.back()->pushConstant.index = meshes.size() - 1;
    meshes.emplace_back(std::make_unique<Mesh>());
    meshes.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
    meshes.back()->pushConstant.index = meshes.size() - 1;
    pointLight = std::make_unique<Light>();
}

void Scene::CreateResource(const Device& device)
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer(device.vkPhysicalDevice, device.vkDevice);
        mesh->CreateVertexBuffer(device.vkPhysicalDevice, device.vkDevice);

        mesh->CreateTexture(device);
        mesh->textureImage->CreateSampler(device.vkPhysicalDevice, device.vkDevice);
    }
}
