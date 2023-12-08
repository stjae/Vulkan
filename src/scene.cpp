#include "scene.h"

Scene::Scene()
{
    // meshes.emplace_back(std::make_shared<Mesh>());
    // meshes.back()->CreateCube(nullptr);
    // meshes.back()->pushConstant.index = meshes.size() - 1;
    // meshNames.push_back(meshes.back()->name.c_str());
    meshes.emplace_back(std::make_shared<Mesh>());
    meshes.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
    meshes.back()->pushConstant.index = meshes.size() - 1;
    meshNames.push_back(meshes.back()->name.c_str());
}

void Scene::Prepare()
{
    camera = std::make_unique<Camera>();
    CreateResource();

    Command command;
    command.CreateCommandPool("copying buffers");

    for (auto& mesh : meshes) {
        command.RecordCopyCommands(mesh->vertexStagingBuffer->GetBuffer(), mesh->vertexBuffer->GetBuffer(), sizeof(mesh->vertices[0]) * mesh->vertices.size());
        command.RecordCopyCommands(mesh->indexStagingBuffer->GetBuffer(), mesh->indexBuffer->GetBuffer(), sizeof(mesh->indices[0]) * mesh->indices.size());
        command.TransitImageLayout(mesh->textureImage->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    }

    command.Submit();

    for (auto& mesh : meshes)
        command.RecordCopyCommands(mesh->textureStagingBuffer->GetBuffer(), mesh->textureImage->GetImage(), mesh->textureWidth, mesh->textureHeight, mesh->textureSize);

    command.Submit();

    for (auto& mesh : meshes) {
        command.TransitImageLayout(mesh->textureImage->GetImage(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->DestroyStagingBuffer();
    }

    command.Submit();
}

void Scene::CreateResource()
{
    for (auto& mesh : meshes) {
        mesh->CreateIndexBuffer();
        mesh->CreateVertexBuffer();

        mesh->CreateTexture();
        mesh->textureImage->CreateSampler();

        BufferInput input = { sizeof(UBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        mesh->matrixUniformBuffer = std::make_unique<Buffer>(input);
        mesh->matrixUniformBuffer->Map();
    }
}

void Scene::Update(uint32_t index)
{
    if (camera->isControllable) {
        camera->Update();
    }

    camera->matrix_.view = glm::lookAt(camera->pos, camera->at, camera->up);
    camera->matrix_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::GetDetail().extent.width) / static_cast<float>(Swapchain::GetDetail().extent.height), 0.1f, 100.0f);

    vk::WriteDescriptorSet matrixWriteInfo(Swapchain::GetDetail().frames[index].descriptorSets[0], 0, 0, static_cast<uint32_t>(meshes.size()), vk::DescriptorType::eUniformBuffer, nullptr, &camera->GetBufferInfo(), nullptr, nullptr);
    Device::GetDevice().updateDescriptorSets(matrixWriteInfo, nullptr);

    // vk::WriteDescriptorSet descriptorWrites;
    // descriptorWrites.dstSet = Swapchain::GetDetail().frames[index].descriptorSets[0];
    // descriptorWrites.dstBinding = 2;
    // descriptorWrites.dstArrayElement = 0;
    // descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    // descriptorWrites.descriptorCount = meshes.size();
    // std::vector<vk::DescriptorImageInfo> infos;
    // for (auto& mesh : meshes) {
    //     infos.push_back(mesh->textureImage->GetInfo());
    // }
    // descriptorWrites.pImageInfo = infos.data();
    // if (meshes.size() > 0) {
    //     Device::GetDevice().updateDescriptorSets(descriptorWrites, nullptr);
    // }
}