#include "scene.h"
extern size_t dynamicBufferAlignment;

Scene::Scene()
{
    meshes.emplace_back(std::make_shared<Mesh>());
    meshes.back()->CreateCube(nullptr);
    meshes.emplace_back(std::make_shared<Mesh>());
    meshes.back()->LoadModel("models/viking_room.obj", "textures/viking_room.png");
}

void Scene::SetDynamicBufferAlignmentSize()
{
    size_t minUboAlignment = Device::limits.minUniformBufferOffsetAlignment;
    dynamicBufferAlignment = sizeof(glm::mat4);

    if (minUboAlignment > 0) {
        dynamicBufferAlignment = (dynamicBufferAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
}

void Scene::PrepareScene()
{
    SetDynamicBufferAlignmentSize();

    camera = std::make_unique<Camera>();

    for (auto& mesh : meshes) {
        mesh->CreateBuffers();
    }

    if (meshes.size() > 0)
        PrepareMeshes();
}

void Scene::PrepareMeshes()
{
    Command command;
    command.CreateCommandPool("copying buffers");

    for (auto& mesh : meshes) {
        command.RecordCopyCommands(mesh->vertexStagingBuffer->GetHandle().buffer,
                                   mesh->vertexBuffer->GetHandle().buffer,
                                   sizeof(Vertex) * mesh->vertices.size());
        command.RecordCopyCommands(mesh->indexStagingBuffer->GetHandle().buffer,
                                   mesh->indexBuffer->GetHandle().buffer,
                                   sizeof(uint32_t) * mesh->indices.size());
        command.TransitImageLayout(mesh->textureImage->GetHandle().image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal);
    }

    command.Submit();

    for (auto& mesh : meshes)
        command.RecordCopyCommands(mesh->textureStagingBuffer->GetHandle().buffer,
                                   mesh->textureImage->GetHandle().image, mesh->textureWidth,
                                   mesh->textureHeight, mesh->textureSize);

    command.Submit();

    for (auto& mesh : meshes) {
        command.TransitImageLayout(mesh->textureImage->GetHandle().image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->DestroyStagingBuffer();
    }

    command.Submit();

    CreateUniformBuffers();
}

void Scene::CreateUniformBuffers()
{
    if (matrixUniformBufferDynamic_ != nullptr) {
        matrixUniformBufferDynamic_.~unique_ptr();
    }

    size_t bufferSize = meshes.size() * dynamicBufferAlignment;
    uboDataDynamic_.model = (glm::mat4*)AlignedAlloc(dynamicBufferAlignment, bufferSize);

    if (uboDataDynamic_.model == nullptr) {
        spdlog::error("failed to allocate memory for dynamic buffer");
    }

    for (int i = 0; i < meshes.size(); i++) {
        glm::mat4* modelMat = (glm::mat4*)((uint64_t)uboDataDynamic_.model + (i * dynamicBufferAlignment));
        *modelMat = glm::mat4(1.0f);
    }

    BufferInput input = { meshes.size() * dynamicBufferAlignment,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible };
    matrixUniformBufferDynamic_ = std::make_unique<Buffer>(input);
    matrixUniformBufferDynamic_->MapMemory(dynamicBufferAlignment);
}

void Scene::Update(uint32_t index)
{
    if (camera->isControllable) {
        camera->Update();
    }

    camera->matrix_.view = glm::lookAt(camera->pos, camera->at, camera->up);
    camera->matrix_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::GetDetail().extent.width) / static_cast<float>(Swapchain::GetDetail().extent.height), 0.1f, 100.0f);
    camera->UpdateBuffer();

    vk::WriteDescriptorSet cameraMatrixWrite(Swapchain::GetDetail().frames[index].descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &camera->GetBufferInfo(), nullptr, nullptr);
    Device::GetHandle().device.updateDescriptorSets(cameraMatrixWrite, nullptr);

    if (meshes.size() > 0) {
        UpdateBuffer();
        vk::MappedMemoryRange memoryRange;
        memoryRange.memory = matrixUniformBufferDynamic_->GetHandle().bufferMemory;
        size_t size = sizeof(uboDataDynamic_);
        size_t atomSize = Device::GetHandle().physicalDevice.getProperties().limits.nonCoherentAtomSize;
        if (size < atomSize)
            size = atomSize;
        memoryRange.size = size;
        auto result = Device::GetHandle().device.flushMappedMemoryRanges(1, &memoryRange);

        vk::WriteDescriptorSet modelMatrixWrite(
            Swapchain::GetDetail().frames[index].descriptorSets[1], 0, 0, 1,
            vk::DescriptorType::eUniformBufferDynamic, nullptr,
            &matrixUniformBufferDynamic_->GetHandle().bufferInfo, nullptr, nullptr);
        Device::GetHandle().device.updateDescriptorSets(modelMatrixWrite, nullptr);

        vk::WriteDescriptorSet descriptorWrites;
        descriptorWrites.dstSet = Swapchain::GetDetail().frames[index].descriptorSets[2];
        descriptorWrites.dstBinding = 0;
        descriptorWrites.dstArrayElement = 0;
        descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites.descriptorCount = meshes.size();
        std::vector<vk::DescriptorImageInfo> infos;
        for (auto& mesh : meshes) {
            infos.push_back(mesh->textureImage->GetHandle().imageInfo);
        }
        descriptorWrites.pImageInfo = infos.data();
        Device::GetHandle().device.updateDescriptorSets(descriptorWrites, nullptr);
    }
}

void Scene::UpdateBuffer()
{
    matrixUniformBufferDynamic_->UpdateBuffer(uboDataDynamic_.model, matrixUniformBufferDynamic_->GetSize());
}

Scene::~Scene()
{
    AlignedFree(uboDataDynamic_.model);
}