#include "scene.h"

Scene::Scene() : selectedMeshID(-1), selectedMeshInstanceID(-1), selectedLightID(-1), meshDirtyFlag(true), lightDirtyFlag(true), shadowMapDirtyFlag_(true), showLightIcon_(true)
{
    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);
    Command::AllocateCommandBuffer(commandPool_, Mesh::commandBuffer_);

    camera.dir_ = { 0.5, -0.3, -0.7, 0.0 };
    camera.pos_ = { -3.0, 3.3, 8.0 };
    camera.at_ = { -2.5, 3.0, 7.3 };

    CreateDummyTexture();
    CreateShadowMapRenderPass();
    shadowMaps_.reserve(100);
    shadowMaps_.emplace_back();
    shadowMaps_.back().PrepareShadowCubeMap(commandBuffer_);

    BufferInput bufferInput = { sizeof(CameraData), sizeof(CameraData), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    shadowMapCameraBuffer = std::make_unique<Buffer>(bufferInput);
    shadowMapPipeline.cameraDescriptor = shadowMapCameraBuffer->GetBundle().descriptorBufferInfo;
    bufferInput = { sizeof(LightData), sizeof(LightData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    lightDataBuffer_ = std::make_unique<Buffer>(bufferInput);
    meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
    shadowMapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
    bufferInput = { sizeof(InstanceData), sizeof(InstanceData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    meshInstanceDataBuffer_ = std::make_unique<Buffer>(bufferInput);
    meshRenderPipeline.meshDescriptor = meshInstanceDataBuffer_->GetBundle().descriptorBufferInfo;
    shadowMapPipeline.meshDescriptor = meshInstanceDataBuffer_->GetBundle().descriptorBufferInfo;

    meshes.emplace_back(MESHTYPE::SQUARE);
    meshes.back().CreateBuffers();
    meshes.emplace_back(MESHTYPE::CUBE);
    meshes.back().CreateBuffers();
    meshes.emplace_back(MESHTYPE::SPHERE);
    meshes.back().CreateBuffers();

    AddMeshInstance(MESHTYPE::CUBE, glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(10.0f, 0.1f, 10.0f));
    AddMeshInstance(MESHTYPE::SPHERE, glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.3f));
    lights.emplace_back();
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

void Scene::AddMeshInstance(uint32_t id, glm::vec3 pos, glm::vec3 scale)
{
    meshes[id].instanceData_.emplace_back(id, meshes[id].instanceID, pos, scale);
    meshes[id].instanceID++;
    meshDirtyFlag = true;
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

    BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    Buffer stagingBuffer(bufferInput);
    stagingBuffer.Copy(imageData);

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
                               stagingBuffer.GetBundle().buffer,
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
}

void Scene::CreateDummyTexture()
{
    textures.emplace_back(std::make_shared<Texture>());

    std::array<uint8_t, 4> dummyTexture = { 0, 0, 0, 255 };
    textures.back()->width = 1;
    textures.back()->height = 1;
    textures.back()->size = sizeof(dummyTexture);

    BufferInput bufferInput = { sizeof(dummyTexture), sizeof(dummyTexture), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    Buffer stagingBuffer(bufferInput);
    stagingBuffer.Copy(&dummyTexture);

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
                               stagingBuffer.GetBundle().buffer,
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
}

void Scene::Update()
{
    // Duplicate Light
    if (selectedLightID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !camera.isControllable_) {
        AddLight();
        lights.back() = lights[selectedLightID];
    }

    // Duplicate Mesh
    if (selectedMeshID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !camera.isControllable_) {
        AddMeshInstance(selectedMeshID);
        meshes[selectedMeshID].instanceData_.back() = meshes[selectedMeshID].instanceData_[selectedMeshInstanceID];
        meshes[selectedMeshID].instanceData_.back().instanceID++;
    }

    camera.Update();
    if (selectedMeshID > -1 && selectedMeshInstanceID > -1 && ImGui::IsKeyDown(ImGuiKey_F)) {
        camera.pos_ = glm::translate(meshes[selectedMeshID].instanceData_[selectedMeshInstanceID].model, glm::vec3(0.0f, 0.0f, 2.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    camera.cameraData.view = glm::lookAt(camera.pos_, camera.at_, camera.up_);
    camera.cameraData.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.width) / static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.height), 0.1f, 1024.0f);
    camera.cameraData.pos = camera.pos_;
    camera.cameraBuffer_->Copy(&camera.cameraData);

    BufferInput bufferInput;
    for (auto& light : lights)
        light.maxLights = lights.size();
    if (!lights.empty() && lightDirtyFlag) {
        bufferInput = { sizeof(LightData) * lights.size(), sizeof(LightData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_.reset();
        lightDataBuffer_ = std::make_unique<Buffer>(bufferInput);
        lightDataBuffer_->Copy(lights.data());
        meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        shadowMapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        std::vector<vk::WriteDescriptorSet> light = {
            { meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &meshRenderPipeline.lightDescriptor },
            { shadowMapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &shadowMapPipeline.lightDescriptor }
        };
        Device::GetBundle().device.updateDescriptorSets(light, nullptr);
        lightDirtyFlag = false;
        shadowMapDirtyFlag_ = true;
    }

    if (GetInstanceCount() > 0 && meshDirtyFlag) {
        bufferInput = { sizeof(InstanceData) * GetInstanceCount(), sizeof(InstanceData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        meshInstanceDataBuffer_.reset();
        meshInstanceDataBuffer_ = std::make_unique<Buffer>(bufferInput);

        std::vector<InstanceData> instances;
        instances.reserve(GetInstanceCount());
        for (auto& mesh : meshes) {
            instances.insert(instances.end(), mesh.instanceData_.begin(), mesh.instanceData_.end());
        }
        meshInstanceDataBuffer_->Copy(instances.data());
        meshRenderPipeline.meshDescriptor = meshInstanceDataBuffer_->GetBundle().descriptorBufferInfo;
        shadowMapPipeline.meshDescriptor = meshInstanceDataBuffer_->GetBundle().descriptorBufferInfo;
        std::vector<vk::WriteDescriptorSet> mesh = {
            { meshRenderPipeline.descriptorSets[0], 2, 0, 1, vk::DescriptorType::eStorageBufferDynamic, nullptr, &meshRenderPipeline.meshDescriptor },
            { shadowMapPipeline.descriptorSets[0], 2, 0, 1, vk::DescriptorType::eStorageBufferDynamic, nullptr, &shadowMapPipeline.meshDescriptor }
        };
        Device::GetBundle().device.updateDescriptorSets(mesh, nullptr);
        meshDirtyFlag = false;
        shadowMapDirtyFlag_ = true;
    }

    shadowMapCameraData.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1024.f);
    shadowMapCameraBuffer->Copy(&shadowMapCameraData);

    if (shadowMapDirtyFlag_) {
        for (int i = 0; i < lights.size(); i++) {
            shadowMaps_[i].DrawShadowMap(commandBuffer_, i, lights, meshes);
        }
        shadowMapDirtyFlag_ = false;
    }

    std::vector<vk::DescriptorImageInfo> textureInfos;
    for (auto& texture : textures) {
        textureInfos.push_back(texture->image->GetBundle().imageInfo);
    }
    std::vector<vk::DescriptorImageInfo> cubeMapInfos;
    for (auto& cubeMapInfo : meshRenderPipeline.shadowCubeMapDescriptors)
        cubeMapInfos.push_back(cubeMapInfo);
    std::vector<vk::WriteDescriptorSet> image = {
        { meshRenderPipeline.descriptorSets[1], 0, 0, (uint32_t)textureInfos.size(), vk::DescriptorType::eCombinedImageSampler, textureInfos.data() },
        { meshRenderPipeline.descriptorSets[2], 0, 0, (uint32_t)cubeMapInfos.size(), vk::DescriptorType::eCombinedImageSampler, cubeMapInfos.data() }
    };
    Device::GetBundle().device.updateDescriptorSets(image, nullptr);
}

void Scene::AddLight()
{
    lights.emplace_back();
    shadowMaps_.emplace_back();
    shadowMaps_.back().PrepareShadowCubeMap(commandBuffer_);
    lightDirtyFlag = true;
}

void Scene::DeleteMesh()
{
    if (selectedMeshID < 0 || selectedMeshInstanceID < 0)
        return;
    meshes[selectedMeshID].instanceData_.erase(meshes[selectedMeshID].instanceData_.begin() + selectedMeshInstanceID);

    for (int32_t i = selectedMeshInstanceID; i < meshes[selectedMeshID].instanceData_.size(); i++) {
        meshes[selectedMeshID].instanceData_[i].instanceID--;
    }
    meshes[selectedMeshID].instanceID--;

    selectedMeshID = -1;
    selectedMeshInstanceID = -1;
    meshDirtyFlag = true;
}

void Scene::DeleteLight()
{
    if (selectedLightID < 0)
        return;
    lights.erase(lights.begin() + selectedLightID);

    selectedLightID = -1;
    lightDirtyFlag = true;
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
