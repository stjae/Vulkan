#include "scene.h"

Scene::Scene()
{
    command_.CreateCommandPool();

    camera_ = std::make_unique<Camera>();
    uboDataDynamic_.alignment = Buffer::GetDynamicBufferOffset(sizeof(glm::mat4));

    for (auto& mesh : meshes) {
        mesh->CreateBuffers();
    }

    if (!meshes.empty())
        PrepareMeshes();
}

void Scene::PrepareMeshes()
{
    for (auto& mesh : meshes) {
        command_.RecordCopyCommands(mesh->vertexStagingBuffer->GetHandle().buffer,
                                    mesh->vertexBuffer->GetHandle().buffer,
                                    sizeof(Vertex) * mesh->vertices.size());
        command_.RecordCopyCommands(mesh->indexStagingBuffer->GetHandle().buffer,
                                    mesh->indexBuffer->GetHandle().buffer,
                                    sizeof(uint32_t) * mesh->indices.size());
        command_.TransitImageLayout(mesh->textureImage->GetHandle().image,
                                    vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal);
    }

    command_.Submit();

    for (auto& mesh : meshes)
        command_.RecordCopyCommands(mesh->textureStagingBuffer->GetHandle().buffer,
                                    mesh->textureImage->GetHandle().image, mesh->textureWidth,
                                    mesh->textureHeight);

    command_.Submit();

    for (auto& mesh : meshes) {
        command_.TransitImageLayout(mesh->textureImage->GetHandle().image,
                                    vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh->DestroyStagingBuffer();
    }

    command_.Submit();

    CreateUniformBuffers();
}

void Scene::UpdateMesh()
{
    auto& mesh = meshes.back();

    command_.RecordCopyCommands(mesh->vertexStagingBuffer->GetHandle().buffer,
                                mesh->vertexBuffer->GetHandle().buffer,
                                sizeof(Vertex) * mesh->vertices.size());
    command_.RecordCopyCommands(mesh->indexStagingBuffer->GetHandle().buffer,
                                mesh->indexBuffer->GetHandle().buffer,
                                sizeof(uint32_t) * mesh->indices.size());
    command_.TransitImageLayout(mesh->textureImage->GetHandle().image,
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal);

    command_.Submit();

    command_.RecordCopyCommands(mesh->textureStagingBuffer->GetHandle().buffer,
                                mesh->textureImage->GetHandle().image, mesh->textureWidth,
                                mesh->textureHeight);

    command_.Submit();

    command_.TransitImageLayout(mesh->textureImage->GetHandle().image,
                                vk::ImageLayout::eTransferDstOptimal,
                                vk::ImageLayout::eShaderReadOnlyOptimal);
    mesh->textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
    mesh->DestroyStagingBuffer();

    command_.Submit();

    CreateUniformBuffers();
}

void Scene::CreateUniformBuffers()
{
    size_t bufferSize = meshes.size() * uboDataDynamic_.alignment;
    void* newAlignedMemory = AlignedAlloc(uboDataDynamic_.alignment, bufferSize);

    if (matrixUniformBufferDynamic_ != nullptr) {
        for (int i = 0; i < meshes.size() - 1; i++) {
            auto* prevMat = (glm::mat4*)((uint64_t)uboDataDynamic_.model + (i * uboDataDynamic_.alignment));
            auto* currentMat = (glm::mat4*)((uint64_t)newAlignedMemory + (i * uboDataDynamic_.alignment));

            *currentMat = *prevMat;
        }
        AlignedFree(uboDataDynamic_.model);
        matrixUniformBufferDynamic_.~unique_ptr();
    }

    uboDataDynamic_.model = (glm::mat4*)newAlignedMemory;

    if (uboDataDynamic_.model == nullptr) {
        spdlog::error("failed to allocate memory for dynamic buffer");
    }

    size_t indexLastMesh = meshes.size() - 1;
    auto* modelMat = (glm::mat4*)((uint64_t)uboDataDynamic_.model + (indexLastMesh * uboDataDynamic_.alignment));
    *modelMat = glm::mat4(1.0f);

    BufferInput input = { meshes.size() * uboDataDynamic_.alignment,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible };
    matrixUniformBufferDynamic_ = std::make_unique<Buffer>(input);
    matrixUniformBufferDynamic_->MapMemory(uboDataDynamic_.alignment);
}

void Scene::Update(uint32_t frameIndex, const std::vector<SwapchainFrame>& swapchainFrames)
{
    if (camera_->isControllable) {
        camera_->Update();
    }

    camera_->matrix_.view = glm::lookAt(camera_->pos, camera_->at, camera_->up);
    camera_->matrix_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(Swapchain::Get().swapchainImageExtent.height), 0.1f, 100.0f);
    camera_->UpdateBuffer();

    vk::WriteDescriptorSet cameraMatrixWrite(swapchainFrames[frameIndex].descriptor.descriptorSets_[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &camera_->GetBufferInfo(), nullptr, nullptr);
    Device::GetHandle().device.updateDescriptorSets(cameraMatrixWrite, nullptr);

    if (!meshes.empty()) {
        UpdateBuffer();
        vk::MappedMemoryRange memoryRange;
        memoryRange.memory = matrixUniformBufferDynamic_->GetHandle().bufferMemory;
        size_t size = sizeof(uboDataDynamic_);
        size_t atomSize = Device::GetHandle().physicalDevice.getProperties().limits.nonCoherentAtomSize;
        if (size < atomSize)
            size = atomSize;
        memoryRange.size = size;
        if (Device::GetHandle().device.flushMappedMemoryRanges(1, &memoryRange) != vk::Result::eSuccess) {
            spdlog::error("failed to flush mapped memory ranges");
        }

        vk::WriteDescriptorSet modelMatrixWrite(
            swapchainFrames[frameIndex].descriptor.descriptorSets_[1], 0, 0, 1,
            vk::DescriptorType::eUniformBufferDynamic, nullptr,
            &matrixUniformBufferDynamic_->GetHandle().bufferInfo, nullptr, nullptr);
        Device::GetHandle().device.updateDescriptorSets(modelMatrixWrite, nullptr);

        vk::WriteDescriptorSet descriptorWrites;
        descriptorWrites.dstSet = swapchainFrames[frameIndex].descriptor.descriptorSets_[2];
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