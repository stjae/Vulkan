#include "scene.h"

Scene::Scene()
{
    meshes_.emplace_back(std::make_unique<Mesh>());
    meshes_.back()->CreateCube(nullptr);
    meshes_.back()->pushConstant.index = meshes_.size() - 1;
    meshes_.emplace_back(std::make_unique<Mesh>());
    meshes_.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
    meshes_.back()->pushConstant.index = meshes_.size() - 1;
}

void Scene::Prepare(const Device& device)
{
    CreateResource(device);

    Command command(device);
    command.CreateCommandPool("copying buffers");

    for (auto& mesh : meshes_) {
        command.RecordCopyCommands(mesh->vertexStagingBuffer->GetBuffer(), mesh->vertexBuffer->GetBuffer(), sizeof(mesh->vertices[0]) * mesh->vertices.size());
        command.RecordCopyCommands(mesh->indexStagingBuffer->GetBuffer(), mesh->indexBuffer->GetBuffer(), sizeof(mesh->indices[0]) * mesh->indices.size());
        command.TransitImageLayout(mesh->textureImage->image_, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    }

    command.Submit();

    for (auto& mesh : meshes_)
        command.RecordCopyCommands(mesh->textureStagingBuffer->GetBuffer(), mesh->textureImage->image_, mesh->textureWidth, mesh->textureHeight, mesh->textureSize);

    command.Submit();

    for (auto& mesh : meshes_) {
        command.TransitImageLayout(mesh->textureImage->image_, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
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

        BufferInput input = { input.size = sizeof(UBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        mesh->matrixUniformBuffer = std::make_unique<Buffer>(device.vkPhysicalDevice, device.vkDevice, input);
        mesh->matrixUniformBuffer->MapUniformBuffer();
    }
}

void Scene::Update(uint32_t index, const Swapchain& swapchain, const vk::Device& vkDevice, GLFWwindow* window)
{
    if (camera_.isControllable) {
        camera_.Update(window);
    }

    camera_.matrix.view = glm::lookAt(camera_.pos, camera_.at, camera_.up);
    camera_.matrix.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchain.detail.extent.width) / static_cast<float>(swapchain.detail.extent.height), 0.1f, 100.0f);

    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;

    for (auto& mesh : meshes_) {
        mesh->ubo.view = camera_.matrix.view;
        mesh->ubo.proj = camera_.matrix.proj;
        mesh->ubo.eye = camera_.pos;

        mesh->matrixUniformBuffer->UpdateResource(&(mesh->ubo), sizeof(UBO));
        descriptorBufferInfos.push_back(mesh->matrixUniformBuffer->GetBufferInfo());
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
        infos.push_back(mesh->textureImage->GetInfo());
    }
    descriptorWrites.pImageInfo = infos.data();
    if (meshes_.size() > 0) {
        vkDevice.updateDescriptorSets(descriptorWrites, nullptr);
    }
}