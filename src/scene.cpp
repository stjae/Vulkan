#include "scene.h"

Scene::Scene()
{
    // meshes.emplace_back(std::make_unique<Mesh>());
    // meshes.back()->CreateCube(nullptr);
    // meshes.back()->pushConstant.index = meshes.size() - 1;
    // meshes.emplace_back(std::make_unique<Mesh>());
    // meshes.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
    // meshes.back()->pushConstant.index = meshes.size() - 1;
}

void Scene::Prepare(const Device& device)
{
    CreateResource(device);

    Command command(device);
    command.CreateCommandPool("copying buffers");

    for (auto& mesh : meshes_) {
        command.RecordCopyCommands(mesh->vertexStagingBuffer->vkBuffer, mesh->vertexBuffer->vkBuffer, sizeof(mesh->vertices[0]) * mesh->vertices.size());
        command.RecordCopyCommands(mesh->indexStagingBuffer->vkBuffer, mesh->indexBuffer->vkBuffer, sizeof(mesh->indices[0]) * mesh->indices.size());
        command.TransitImageLayout(mesh->textureImage->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    }

    command.Submit();

    for (auto& mesh : meshes_)
        command.RecordCopyCommands(mesh->textureStagingBuffer->vkBuffer, mesh->textureImage->image, mesh->textureWidth, mesh->textureHeight, mesh->textureSize);

    command.Submit();

    for (auto& mesh : meshes_) {
        command.TransitImageLayout(mesh->textureImage->image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetTextureImageInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->DestroyStagingBuffer();
    }

    command.Submit();
}

void Scene::CreateResource(const Device& device)
{
    for (auto& mesh : meshes_) {
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

void Scene::Update(uint32_t index, const Swapchain& swapchain, const vk::Device& vkDevice)
{
    if (camera_.isControllable) {
        camera_.Update();
    }

    camera_.matrix.view = glm::lookAt(camera_.pos, camera_.at, camera_.up);
    camera_.matrix.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.detail.extent.width) / static_cast<float>(swapchain.detail.extent.height), 0.1f, 100.0f);

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;

    for (auto& mesh : meshes_) {
        mesh->ubo.view = camera_.matrix.view;
        mesh->ubo.proj = camera_.matrix.proj;
        mesh->ubo.eye = camera_.pos;

        memcpy(mesh->matrixUniformBuffer->memory.memoryLocation, &(mesh->ubo), sizeof(UBO));

        descriptorBufferInfos.push_back(mesh->matrixUniformBuffer->descriptorBufferInfo);
    }

    if (meshes_.size() > 0) {
        vk::WriteDescriptorSet matrixWriteInfo(swapchain.detail.frames[index].descriptorSets[0], 0, 0, meshes_.size(), vk::DescriptorType::eUniformBuffer, nullptr, descriptorBufferInfos.data(), nullptr, nullptr);
        vkDevice.updateDescriptorSets(matrixWriteInfo, nullptr);
    }

    vk::WriteDescriptorSet descriptorWrites;
    descriptorWrites.dstSet = swapchain.detail.frames[index].descriptorSets[0];
    descriptorWrites.dstBinding = 2;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites.descriptorCount = meshes_.size();
    std::vector<vk::DescriptorImageInfo> infos;
    for (auto& mesh : meshes_) {
        infos.push_back(mesh->textureImage->imageInfo);
    }
    descriptorWrites.pImageInfo = infos.data();
    if (meshes_.size() > 0) {
        vkDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
}