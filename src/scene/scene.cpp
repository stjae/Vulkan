#include "scene.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Scene::Scene()
{
    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    meshDynamicUniformBufferRange_ = Buffer::GetDynamicBufferRange(sizeof(MeshUniformData));
    CreateDummyTexture();

    meshes.emplace_back();
    meshes.back().CreateSphere(0.05f, glm::vec3(1.0f, 1.0f, 0.0f), "light");
    meshes.back().position_ = glm::vec3(0.0f, 2.0f, 2.0f);
    meshes.back().CreateBuffers();
    PrepareMesh(meshes.back());

    meshes.emplace_back();
    meshes.back().CreateSphere();
    meshes.back().CreateBuffers();
    PrepareMesh(meshes.back());
}

void Scene::AddMesh(TypeEnum::Mesh type)
{
    switch (type) {
    case TypeEnum::Mesh::SQUARE:
        meshes.emplace_back();
        meshes.back().CreateSquare();
        meshes.back().CreateBuffers();
        meshes.back().meshType_ = TypeEnum::Mesh::SQUARE;
        break;
    case TypeEnum::Mesh::CUBE:
        meshes.emplace_back();
        meshes.back().CreateCube();
        meshes.back().CreateBuffers();
        meshes.back().meshType_ = TypeEnum::Mesh::CUBE;
        break;
    case TypeEnum::Mesh::SPHERE:
        meshes.emplace_back();
        meshes.back().CreateSphere();
        meshes.back().CreateBuffers();
        meshes.back().meshType_ = TypeEnum::Mesh::CUBE;
        break;
    default:
        break;
    }

    PrepareMesh(meshes.back());
}

void Scene::AddMesh(TypeEnum::Mesh type, const std::string& filePath)
{
    if (filePath.empty())
        return;

    meshes.emplace_back();
    meshes.back().LoadModel(filePath);
    meshes.back().CreateBuffers();
    meshes.back().meshType_ = type;

    PrepareMesh(meshes.back());
}

void Scene::AddTexture(const std::string& filePath)
{
    int width, height, channel;
    vk::DeviceSize imageSize;
    stbi_uc* imageData;

    imageData = stbi_load(filePath.c_str(), &width, &height, &channel, STBI_rgb_alpha);
    imageSize = width * height * 4;

    if (!imageData) {
        spdlog::error("failed to load texture");
        return;
    }

    textures.emplace_back(std::make_shared<Texture>());
    auto& texture = textures.back();
    texture->width = width;
    texture->height = height;
    texture->size = static_cast<size_t>(imageSize);
    texture->index = textures.size() - 1;

    BufferInput input = { imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    texture->stagingBuffer = std::make_unique<Buffer>(input);
    texture->stagingBuffer->CopyResourceToBuffer(imageData, input);

    stbi_image_free(imageData);
    vk::Extent3D extent(width, height, 1);
    texture->image = std::make_unique<Image>();
    texture->image->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent, vk::ImageTiling::eOptimal);
    texture->image->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    texture->image->CreateSampler();
    texture->image->SetInfo();

    Command::Begin(commandBuffer_);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   *textures.back()->image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer_,
                               textures.back()->stagingBuffer->GetBundle().buffer,
                               textures.back()->image->GetBundle().image, textures.back()->width,
                               textures.back()->height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   *textures.back()->image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    textures.back()->stagingBuffer->Destroy();
}

void Scene::PrepareMesh(Mesh& mesh)
{
    Command::Begin(commandBuffer_);
    // Copy vertices from staging buffer
    Command::CopyBufferToBuffer(commandBuffer_,
                                mesh.vertexStagingBuffer->GetBundle().buffer,
                                mesh.vertexBuffer->GetBundle().buffer,
                                sizeof(Vertex) * mesh.vertices.size());

    // Copy indices from staging buffer
    Command::CopyBufferToBuffer(commandBuffer_,
                                mesh.indexStagingBuffer->GetBundle().buffer,
                                mesh.indexBuffer->GetBundle().buffer,
                                sizeof(uint32_t) * mesh.indices.size());
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    mesh.vertexStagingBuffer->Destroy();
    mesh.indexStagingBuffer->Destroy();

    CreateMeshUniformBuffer();
}

void Scene::CreateMeshUniformBuffer()
{
    if (meshes.empty())
        return;

    size_t requiredBufferSize = meshes.size() * meshDynamicUniformBufferRange_;
    Log(debug, fmt::terminal_color::blue, "model uniform buffer size - old: {} | new: {}", meshDynamicUniformBufferSize_, requiredBufferSize);

    // if the modelUniformBuffer already exists
    if (meshDynamicUniformBufferSize_ < requiredBufferSize) {
        void* newAlignedMemory = AlignedAlloc(meshDynamicUniformBufferRange_, requiredBufferSize);
        if (!newAlignedMemory)
            spdlog::error("failed to allocate dynamic uniform buffer memory");

        if (meshDynamicUniformBuffer_ != nullptr) {
            for (int i = 0; i < meshes.size() - 1; i++) {
                auto* oldData = (MeshUniformData*)((uint64_t)meshUniformData + (i * meshDynamicUniformBufferRange_));
                auto* newData = (MeshUniformData*)((uint64_t)newAlignedMemory + (i * meshDynamicUniformBufferRange_));

                *newData = *oldData;
            }
            AlignedFree(meshUniformData);
            meshDynamicUniformBuffer_.reset();
        }

        meshUniformData = (MeshUniformData*)newAlignedMemory;
    }

    size_t newMeshIndex = meshes.size() - 1;
    auto* newMeshData = (MeshUniformData*)((uint64_t)meshUniformData + (newMeshIndex * meshDynamicUniformBufferRange_));

    (*newMeshData).modelMat = glm::translate(glm::mat4(1.0f), meshes.back().position_);
    (*newMeshData).meshID = (int32_t)newMeshIndex;
    (*newMeshData).textureID = 0;
    (*newMeshData).useTexture = false;

    if (meshDynamicUniformBufferSize_ < requiredBufferSize) {
        BufferInput input = { meshes.size() * meshDynamicUniformBufferRange_,
                              vk::BufferUsageFlagBits::eUniformBuffer,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        meshDynamicUniformBuffer_ = std::make_unique<Buffer>(input);
        meshDynamicUniformBuffer_->MapMemory(meshDynamicUniformBufferRange_);

        meshDynamicUniformBufferSize_ = requiredBufferSize;
    }
}

void Scene::RearrangeMeshUniformBuffer(size_t index) const
{
    // move the meshUniformData forward as the mesh index refers to has been deleted
    for (size_t i = index; i < meshes.size(); i++) {
        *(MeshUniformData*)((uint64_t)meshUniformData + (i * meshDynamicUniformBufferRange_)) = *(MeshUniformData*)((uint64_t)meshUniformData + ((i + 1) * meshDynamicUniformBufferRange_));
        (*(MeshUniformData*)((uint64_t)meshUniformData + (i * meshDynamicUniformBufferRange_))).meshID--;
    }
}

void Scene::CreateDummyTexture()
{
    textures.emplace_back(std::make_shared<Texture>());

    std::array<uint8_t, 4> dummyTexture = { 0, 0, 0, 255 };
    textures.back()->width = 1;
    textures.back()->height = 1;
    textures.back()->size = sizeof(dummyTexture);

    BufferInput input = { sizeof(dummyTexture), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    textures.back()->stagingBuffer = std::make_unique<Buffer>(input);
    textures.back()->stagingBuffer->CopyResourceToBuffer(&dummyTexture, input);

    vk::Extent3D extent(1, 1, 1);
    textures.back()->image = std::make_unique<Image>();
    textures.back()->image->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent, vk::ImageTiling::eOptimal);
    textures.back()->image->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    textures.back()->image->CreateSampler();
    textures.back()->image->SetInfo();

    Command::Begin(commandBuffer_);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   *textures.back()->image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer_,
                               textures.back()->stagingBuffer->GetBundle().buffer,
                               textures.back()->image->GetBundle().image, textures.back()->width,
                               textures.back()->height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   *textures.back()->image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    textures.back()->stagingBuffer->Destroy();
}

void Scene::Update()
{
    camera.Update();
    camera.matrix_.view = glm::lookAt(camera.pos_, camera.at_, camera.up_);
    camera.matrix_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.width) / static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.height), 0.1f, 100.0f);
    camera.UpdateBuffer();

    light.lightUniformData.modelMat = (*(MeshUniformData*)((uint64_t)meshUniformData)).modelMat;
    light.UpdateBuffer();

    if (!meshes.empty())
        UpdateMeshUniformBuffer();
}

void Scene::UpdateMeshUniformBuffer()
{
    meshDynamicUniformBuffer_->UpdateBuffer(meshUniformData, meshDynamicUniformBuffer_->GetSize());
}

void Scene::DeleteMesh(size_t index)
{
    selectedMeshIndex = -1;
    meshes.erase(meshes.begin() + index);
    RearrangeMeshUniformBuffer(index);
}

Scene::~Scene()
{
    AlignedFree(meshUniformData);
    Device::GetBundle().device.freeCommandBuffers(commandPool_, commandBuffer_);
    Device::GetBundle().device.destroyCommandPool(commandPool_);
}
