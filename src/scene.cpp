#include "scene.h"

Scene::Scene()
{
    meshes.emplace_back(std::make_unique<Mesh>());
    meshes.back()->CreateCube(nullptr);
    meshes.back()->pushConstant.index = meshes.size() - 1;
    meshes.emplace_back(std::make_unique<Mesh>());
    meshes.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
    meshes.back()->pushConstant.index = meshes.size() - 1;
}

void Scene::CreateResource(const Device& device)
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer(device.vkPhysicalDevice, device.vkDevice);
        mesh->CreateVertexBuffer(device.vkPhysicalDevice, device.vkDevice);

        mesh->CreateTexture(device);
        mesh->textureImage->CreateSampler(device.vkPhysicalDevice, device.vkDevice);

        BufferInput input;
        input.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        input.size = sizeof(UBO);
        input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
        mesh->matrixUniformBuffer = std::make_unique<Buffer>(device.vkPhysicalDevice, device.vkDevice, input);
        mesh->matrixUniformBuffer->MapUniformBuffer();
    }
}
