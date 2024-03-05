#include "scene.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define PRIMITIVE_TYPE_COUNT 3

Scene::Scene() : meshID(0), selectedMeshID(-1)
{
    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);
    Command::AllocateCommandBuffer(commandPool_, Mesh::commandBuffer_);

    CreateDummyTexture();

    meshes.emplace_back(MESHTYPE::SQUARE);
    meshes.back().CreateBuffers();
    meshes.emplace_back(MESHTYPE::CUBE);
    meshes.back().CreateBuffers();
    meshes.emplace_back(MESHTYPE::SPHERE);
    meshes.back().CreateBuffers();

    // int count = 0;
    // for (int i = 0; i < 100; i++) {
    //     for (int j = 0; j < 100; j++) {
    //         meshes[1].instanceData_.emplace_back(1, meshes[1].instanceID);
    //         meshes[1].instanceID++;
    //         count++;
    //     }
    // }
    // std::cout << count << '\n';
}

void Scene::AddResource(std::string& filePath)
{
    resources.emplace_back(filePath);
    switch (resources.back().resourceType) {
    case RESOURCETYPE::MESH:
        meshes.emplace_back(meshes.size() - 1, resources.back().filePath);
        meshes.back().CreateBuffers();
        meshes.back().meshID++;
        resources.back().resource = &meshes.back();
        break;
    case RESOURCETYPE::TEXTURE:
        AddTexture(filePath);
        textures.back()->descriptorSet = ImGui_ImplVulkan_AddTexture(textures.back()->image->GetBundle().sampler, textures.back()->image->GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        resources.back().resource = textures.back().get();
        break;
    }
}

void Scene::AddMesh(uint32_t id)
{
    meshes[id].instanceData_.emplace_back(id, meshes[id].instanceID);
    meshes[id].instanceID++;
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
    camera.cameraUniformData.view = glm::lookAt(camera.pos_, camera.at_, camera.up_);
    camera.cameraUniformData.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.width) / static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.height), 0.1f, 100.0f);
    camera.cameraUniformData.pos = camera.pos_;
    camera.UpdateBuffer();

    // light.lightUniformData.modelMat = (*(MeshUniformData*)((uint64_t)meshDynamicUniformBuffer_.data)).model;
    // light.UpdateBuffer();

    if (meshes.empty() || GetInstanceCount() < 1)
        return;

    BufferInput bufferInput = { sizeof(InstanceData) * GetInstanceCount(), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    instanceDataBuffer_.reset();
    instanceDataBuffer_ = std::make_unique<Buffer>(bufferInput);

    std::vector<InstanceData> instances;
    instances.reserve(GetInstanceCount());
    for (auto& mesh : meshes) {
        instances.insert(instances.end(), mesh.instanceData_.begin(), mesh.instanceData_.end());
    }
    instanceDataBuffer_->CopyResourceToBuffer(instances.data(), bufferInput);
}

void Scene::DeleteMesh()
{
    if (selectedMeshID < 0 || selectedInstanceID < 0)
        return;
    meshes[selectedMeshID].instanceData_.erase(meshes[selectedMeshID].instanceData_.begin() + selectedInstanceID);
    selectedMeshID = -1;
    selectedInstanceID = -1;
}

size_t Scene::GetInstanceCount()
{
    size_t instanceCount = 0;
    for (auto& mesh : meshes) {
        instanceCount += mesh.GetInstanceCount();
    }
    return instanceCount;
}

Scene::~Scene()
{
    Device::GetBundle().device.freeCommandBuffers(commandPool_, commandBuffer_);
    Device::GetBundle().device.destroyCommandPool(commandPool_);
}

Resource::Resource(std::string& path)
{
    this->filePath = path;
    this->fileName = path.substr(path.rfind('/') + 1, path.rfind('.') - path.rfind('/') - 1);
    this->fileFormat = path.substr(path.rfind('.') + 1, path.size());

    if (fileFormat == "obj") {
        resourceType = RESOURCETYPE::MESH;
    } else if (fileFormat == "png" || fileFormat == "jpg") {
        resourceType = RESOURCETYPE::TEXTURE;
    }
}
