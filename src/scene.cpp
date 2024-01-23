#include "scene.h"

Scene::Scene()
{
    command_.CreateCommandPool();
    uboDataDynamic.alignment = Buffer::GetDynamicBufferOffset(sizeof(glm::mat4));

    for (auto& mesh : meshes) {
        mesh.CreateBuffers();
    }

    if (!meshes.empty())
        PrepareMeshes();
}

void Scene::AddMesh(MeshType type)
{
    switch (type) {
    case SQUARE:
        meshes.emplace_back();
        meshes.back().CreateSquare(nullptr);
        meshes.back().CreateBuffers();
        meshes.back().meshType_ = SQUARE;
        meshes.back().name_.append(std::to_string(++meshCount_[SQUARE]));
        break;
    case CUBE:
        meshes.emplace_back();
        meshes.back().CreateCube(nullptr);
        meshes.back().CreateBuffers();
        meshes.back().meshType_ = CUBE;
        meshes.back().name_.append(std::to_string(++meshCount_[CUBE]));
        break;
    default:
        break;
    }

    UpdateMesh();
}

void Scene::AddMesh(MeshType type, const std::string& filePath)
{
    if (filePath.empty())
        return;

    meshes.emplace_back();
    meshes.back().LoadModel(filePath, nullptr);
    meshes.back().CreateBuffers();
    meshes.back().meshType_ = type;
    meshes.back().name_.append(std::to_string(++meshCount_[type]));

    UpdateMesh();
}

void Scene::PrepareMeshes()
{
    for (auto& mesh : meshes) {
        command_.AllocateCommandBuffer();
        command_.RecordCopyCommands(mesh.vertexStagingBuffer->GetHandle().buffer,
                                    mesh.vertexBuffer->GetHandle().buffer,
                                    sizeof(Vertex) * mesh.vertices.size());
        command_.AllocateCommandBuffer();
        command_.RecordCopyCommands(mesh.indexStagingBuffer->GetHandle().buffer,
                                    mesh.indexBuffer->GetHandle().buffer,
                                    sizeof(uint32_t) * mesh.indices.size());
        command_.AllocateCommandBuffer();
        command_.TransitImageLayout(mesh.textureImage.get(),
                                    vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eTransferDstOptimal,
                                    {},
                                    vk::AccessFlagBits::eTransferWrite,
                                    vk::PipelineStageFlagBits::eTopOfPipe,
                                    vk::PipelineStageFlagBits::eTransfer);
    }

    command_.Submit();
    Device::GetHandle().device.freeCommandBuffers(command_.commandPool_, command_.commandBuffers_);
    command_.commandBuffers_.clear();

    for (auto& mesh : meshes) {
        command_.AllocateCommandBuffer();
        command_.RecordCopyCommands(mesh.textureStagingBuffer->GetHandle().buffer,
                                    mesh.textureImage->GetHandle().image, mesh.textureWidth,
                                    mesh.textureHeight);
    }

    command_.Submit();
    Device::GetHandle().device.freeCommandBuffers(command_.commandPool_, command_.commandBuffers_);
    command_.commandBuffers_.clear();

    for (auto& mesh : meshes) {
        command_.AllocateCommandBuffer();
        command_.TransitImageLayout(mesh.textureImage.get(),
                                    vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageLayout::eShaderReadOnlyOptimal,
                                    vk::AccessFlagBits::eTransferWrite,
                                    vk::AccessFlagBits::eShaderRead,
                                    vk::PipelineStageFlagBits::eTransfer,
                                    vk::PipelineStageFlagBits::eFragmentShader);
        mesh.textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
        mesh.DestroyStagingBuffer();
    }

    command_.Submit();
    Device::GetHandle().device.freeCommandBuffers(command_.commandPool_, command_.commandBuffers_);
    command_.commandBuffers_.clear();

    CreateUniformBuffer();
}

void Scene::UpdateMesh()
{
    auto& mesh = meshes.back();

    command_.AllocateCommandBuffer();
    command_.RecordCopyCommands(mesh.vertexStagingBuffer->GetHandle().buffer,
                                mesh.vertexBuffer->GetHandle().buffer,
                                sizeof(Vertex) * mesh.vertices.size());
    command_.AllocateCommandBuffer();
    command_.RecordCopyCommands(mesh.indexStagingBuffer->GetHandle().buffer,
                                mesh.indexBuffer->GetHandle().buffer,
                                sizeof(uint32_t) * mesh.indices.size());
    command_.AllocateCommandBuffer();
    command_.TransitImageLayout(mesh.textureImage.get(),
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal,
                                {},
                                vk::AccessFlagBits::eTransferWrite,
                                vk::PipelineStageFlagBits::eTopOfPipe,
                                vk::PipelineStageFlagBits::eTransfer);

    command_.Submit();
    Device::GetHandle().device.freeCommandBuffers(command_.commandPool_, command_.commandBuffers_);
    command_.commandBuffers_.clear();

    command_.AllocateCommandBuffer();
    command_.RecordCopyCommands(mesh.textureStagingBuffer->GetHandle().buffer,
                                mesh.textureImage->GetHandle().image, mesh.textureWidth,
                                mesh.textureHeight);

    command_.Submit();
    Device::GetHandle().device.freeCommandBuffers(command_.commandPool_, command_.commandBuffers_);
    command_.commandBuffers_.clear();

    command_.AllocateCommandBuffer();
    command_.TransitImageLayout(mesh.textureImage.get(),
                                vk::ImageLayout::eTransferDstOptimal,
                                vk::ImageLayout::eShaderReadOnlyOptimal,
                                vk::AccessFlagBits::eTransferWrite,
                                vk::AccessFlagBits::eShaderRead,
                                vk::PipelineStageFlagBits::eTransfer,
                                vk::PipelineStageFlagBits::eFragmentShader);
    mesh.textureImage->SetInfo(vk::ImageLayout::eShaderReadOnlyOptimal);
    mesh.DestroyStagingBuffer();

    command_.Submit();
    Device::GetHandle().device.freeCommandBuffers(command_.commandPool_, command_.commandBuffers_);
    command_.commandBuffers_.clear();

    CreateUniformBuffer();
}

void Scene::CreateUniformBuffer()
{
    size_t bufferSize = meshes.size() * uboDataDynamic.alignment;
    void* newAlignedMemory = AlignedAlloc(uboDataDynamic.alignment, bufferSize);

    if (matrixUniformBufferDynamic_ != nullptr) {
        for (int i = 0; i < meshes.size() - 1; i++) {
            auto* prevMat = (glm::mat4*)((uint64_t)uboDataDynamic.model + (i * uboDataDynamic.alignment));
            auto* currentMat = (glm::mat4*)((uint64_t)newAlignedMemory + (i * uboDataDynamic.alignment));

            *currentMat = *prevMat;
        }

        AlignedFree(uboDataDynamic.model);
        matrixUniformBufferDynamic_.reset();
    }

    uboDataDynamic.model = (glm::mat4*)newAlignedMemory;

    if (uboDataDynamic.model == nullptr) {
        spdlog::error("failed to allocate memory for dynamic buffer");
    }

    size_t indexLastMesh = meshes.size() - 1;
    auto* modelMat = (glm::mat4*)((uint64_t)uboDataDynamic.model + (indexLastMesh * uboDataDynamic.alignment));
    *modelMat = glm::mat4(1.0f);

    BufferInput input = { meshes.size() * uboDataDynamic.alignment,
                          vk::BufferUsageFlagBits::eUniformBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible };
    matrixUniformBufferDynamic_ = std::make_unique<Buffer>(input);
    matrixUniformBufferDynamic_->MapMemory(uboDataDynamic.alignment);
}

void Scene::Update(uint32_t frameIndex, const std::vector<ViewportFrame>& viewportFrames)
{
    if (camera.isControllable_) {
        camera.Update();
    }

    camera.matrix_.view = glm::lookAt(camera.pos_, camera.at_, camera.up_);
    camera.matrix_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(Swapchain::Get().swapchainImageExtent.height), 0.1f, 100.0f);
    camera.UpdateBuffer();

    vk::WriteDescriptorSet cameraMatrixWrite(viewportFrames[frameIndex].descriptor.descriptorSets_[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &camera.GetBufferInfo(), nullptr, nullptr);
    Device::GetHandle().device.updateDescriptorSets(cameraMatrixWrite, nullptr);

    if (!meshes.empty()) {
        UpdateBuffer();
        vk::MappedMemoryRange memoryRange;
        memoryRange.memory = matrixUniformBufferDynamic_->GetHandle().bufferMemory;
        size_t size = sizeof(uboDataDynamic);
        size_t atomSize = Device::GetHandle().physicalDevice.getProperties().limits.nonCoherentAtomSize;
        if (size < atomSize)
            size = atomSize;
        memoryRange.size = size;
        if (Device::GetHandle().device.flushMappedMemoryRanges(1, &memoryRange) != vk::Result::eSuccess) {
            spdlog::error("failed to flush mapped memory ranges");
        }

        vk::WriteDescriptorSet modelMatrixWrite(
            viewportFrames[frameIndex].descriptor.descriptorSets_[1], 0, 0, 1,
            vk::DescriptorType::eUniformBufferDynamic, nullptr,
            &matrixUniformBufferDynamic_->GetHandle().bufferInfo, nullptr, nullptr);
        Device::GetHandle().device.updateDescriptorSets(modelMatrixWrite, nullptr);

        vk::WriteDescriptorSet descriptorWrites;
        descriptorWrites.dstSet = viewportFrames[frameIndex].descriptor.descriptorSets_[2];
        descriptorWrites.dstBinding = 0;
        descriptorWrites.dstArrayElement = 0;
        descriptorWrites.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites.descriptorCount = meshes.size();
        std::vector<vk::DescriptorImageInfo> infos;
        for (auto& mesh : meshes) {
            infos.push_back(mesh.textureImage->GetHandle().imageInfo);
        }
        descriptorWrites.pImageInfo = infos.data();
        Device::GetHandle().device.updateDescriptorSets(descriptorWrites, nullptr);
    }
}

void Scene::UpdateBuffer()
{
    matrixUniformBufferDynamic_->UpdateBuffer(uboDataDynamic.model, matrixUniformBufferDynamic_->GetSize());
}

void Scene::DeleteMesh(long index, MeshType type)
{
    meshes.erase(meshes.begin() + index);
    meshCount_[type]--;
}

Scene::~Scene()
{
    AlignedFree(uboDataDynamic.model);
}
