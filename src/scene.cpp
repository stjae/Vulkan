#include "scene.h"

Scene::Scene()
{
    std::unique_ptr<Mesh> cube = std::make_unique<Mesh>();
    std::unique_ptr<Mesh> square = std::make_unique<Mesh>();
    cube->CreateCube();
    // square->CreateSquare();

    meshes.push_back(std::move(cube));
    // meshes.push_back(std::move(square));

    pointLight = std::make_unique<Light>();
}

void Scene::CreateResource(const Device& device)
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer(device.vkPhysicalDevice, device.vkDevice);
        mesh->CreateVertexBuffer(device.vkPhysicalDevice, device.vkDevice);

        mesh->CreateTexture(device, "/image/box.png");
        mesh->textureImage->CreateSampler(device.vkPhysicalDevice, device.vkDevice);
    }
}
