#include "scene.h"

Scene::Scene() : selectedMeshID_(-1), selectedMeshInstanceID_(-1), selectedLightID_(-1), meshDirtyFlag_(true), lightDirtyFlag_(true), shadowMapDirtyFlag_(true), showLightIcon_(true)
{
    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);
    Command::AllocateCommandBuffer(commandPool_, Mesh::commandBuffer_);

    {
        // init camera pos
        camera_.dir_ = { 0.5, -0.3, -0.7, 0.0 };
        camera_.pos_ = { -3.0, 3.3, 8.0 };
        camera_.at_ = { -2.5, 3.0, 7.3 };
    }

    {
        // prepare textures
        textures_.reserve(100);
        textures_.emplace_back();
        CreateDummyTexture(textures_.back());
        CreateShadowMapRenderPass();
        shadowMaps_.reserve(100);
        shadowMaps_.emplace_back();
        shadowMaps_.back().PrepareShadowCubeMap(commandBuffer_);
        diffuseTextures_.reserve(100);
        diffuseTextures_.emplace_back();
        CreateDummyTexture(diffuseTextures_.back());
        normalTextures_.reserve(100);
        normalTextures_.emplace_back();
        CreateDummyTexture(normalTextures_.back());
    }

    {
        // prepare buffer, descriptor
        BufferInput bufferInput = { sizeof(CameraData), sizeof(CameraData), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        shadowMapCameraBuffer_ = std::make_unique<Buffer>(bufferInput);
        shadowMapPipeline.cameraDescriptor = shadowMapCameraBuffer_->GetBundle().descriptorBufferInfo;
        bufferInput = { sizeof(LightData), sizeof(LightData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_ = std::make_unique<Buffer>(bufferInput);
        meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        shadowMapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
    }

    {
        // init scene
        meshes_.emplace_back(MESHTYPE::SQUARE);
        meshes_.back().CreateBuffers();
        meshes_.emplace_back(MESHTYPE::CUBE);
        meshes_.back().CreateBuffers();
        meshes_.emplace_back(MESHTYPE::SPHERE);
        meshes_.back().CreateBuffers();
        // AddMeshInstance(MESHTYPE::CUBE, glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(10.0f, 0.1f, 10.0f));
        // AddMeshInstance(MESHTYPE::SPHERE, glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.3f));
        lights_.emplace_back(glm::vec3(0.0f, 1.2f, 0.5f));
    }
}

void Scene::AddResource(std::string& filePath)
{
    resources_.emplace_back(filePath);
    switch (resources_.back().resourceType) {
    case RESOURCETYPE::MESH:
        meshes_.emplace_back(meshes_.size() - 1, resources_.back().filePath);
        meshes_.back().CreateBuffers();
        meshes_.back().meshID_++;
        resources_.back().resource = &meshes_.back();
        if (!meshes_.back().materials_.empty()) {
            LoadMaterials(filePath, meshes_.back().materials_);
        }
        break;
    case RESOURCETYPE::TEXTURE:
        textures_.emplace_back();
        CreateTexture(filePath, textures_.back());
        textures_.back().descriptorSet = ImGui_ImplVulkan_AddTexture(textures_.back().image.GetBundle().sampler, textures_.back().image.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        resources_.back().resource = &textures_.back();
        break;
    }
}

void Scene::LoadMaterials(const std::string& modelPath, const std::vector<Material>& materials)
{
    for (auto& material : materials) {
        if (modelPath.find_last_of("/\\") != std::string::npos) {
            std::string texturePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.diffusePath;
            diffuseTextures_.emplace_back();
            CreateTexture(texturePath, diffuseTextures_.back());
            texturePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normalPath;
            normalTextures_.emplace_back();
            CreateTexture(texturePath, normalTextures_.back(), vk::Format::eR8G8B8A8Unorm);
        }
    }
}

void Scene::AddMeshInstance(uint32_t id, glm::vec3 pos, glm::vec3 scale)
{
    meshes_[id].meshInstances_.emplace_back(id, meshes_[id].instanceID_, pos, scale);
    meshes_[id].instanceID_++;
    meshDirtyFlag_ = true;
}

void Scene::CreateTexture(const std::string& filePath, Texture& texture, vk::Format format)
{
    int width = 0, height = 0, channel = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc* imageData = nullptr;

    imageData = stbi_load(filePath.c_str(), &width, &height, &channel, STBI_rgb_alpha);
    imageSize = width * height * 4;

    if (!imageData) {
        spdlog::error("failed to load texture from [{}], inserting dummy", filePath.c_str());
        CreateDummyTexture(texture);
        return;
    }
    spdlog::info("load texture from [{}]", filePath.c_str());

    texture.width = width;
    texture.height = height;
    texture.size = static_cast<size_t>(imageSize);
    // TODO: index
    texture.index = textures_.size() - 1;

    BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    Buffer stagingBuffer(bufferInput);
    stagingBuffer.Copy(imageData);

    stbi_image_free(imageData);
    vk::Extent3D extent(width, height, 1);

    texture.image.CreateImage(format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent, vk::ImageTiling::eOptimal);
    texture.image.CreateImageView(format, vk::ImageAspectFlagBits::eColor);
    texture.image.CreateSampler();
    texture.image.SetInfo();

    Command::Begin(commandBuffer_);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   texture.image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer_,
                               stagingBuffer.GetBundle().buffer,
                               texture.image.GetBundle().image, texture.width,
                               texture.height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   texture.image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);
}

void Scene::CreateDummyTexture(Texture& texture)
{
    std::array<uint8_t, 4> dummyTexture = { 0, 0, 0, 255 };
    texture.width = 1;
    texture.height = 1;
    texture.size = sizeof(dummyTexture);

    BufferInput bufferInput = { sizeof(dummyTexture), sizeof(dummyTexture), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    Buffer stagingBuffer(bufferInput);
    stagingBuffer.Copy(&dummyTexture);

    vk::Extent3D extent(1, 1, 1);
    texture.image.CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent, vk::ImageTiling::eOptimal);
    texture.image.CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    texture.image.CreateSampler();
    texture.image.SetInfo();

    Command::Begin(commandBuffer_);
    // Set texture image layout to transfer dst optimal
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   texture.image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTransfer);
    // Copy texture image from staging buffer
    Command::CopyBufferToImage(commandBuffer_,
                               stagingBuffer.GetBundle().buffer,
                               texture.image.GetBundle().image, texture.width,
                               texture.height);
    // Set texture image layout to shader read only
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   texture.image,
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
    if (selectedLightID_ > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !camera_.isControllable_) {
        AddLight();
        lights_.back() = lights_[selectedLightID_];
    }

    // Duplicate Mesh
    if (selectedMeshID_ > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !camera_.isControllable_) {
        AddMeshInstance(selectedMeshID_);
        int newInstanceID = meshes_[selectedMeshID_].meshInstances_.back().instanceID;
        // copy data of source instance
        auto& srcMeshInstance = meshes_[selectedMeshID_].meshInstances_[selectedMeshInstanceID_];
        meshes_[selectedMeshID_].meshInstances_.back() = srcMeshInstance;
        // except for id
        meshes_[selectedMeshID_].meshInstances_.back().instanceID = newInstanceID;
        // select new instance
        selectedMeshInstanceID_ = newInstanceID;
    }

    camera_.Update();
    if (selectedMeshID_ > -1 && selectedMeshInstanceID_ > -1 && ImGui::IsKeyDown(ImGuiKey_F)) {
        camera_.pos_ = glm::translate(meshes_[selectedMeshID_].meshInstances_[selectedMeshInstanceID_].model, glm::vec3(0.0f, 0.0f, 2.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    camera_.cameraData.view = glm::lookAt(camera_.pos_, camera_.at_, camera_.up_);
    camera_.cameraData.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.width) / static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.height), 0.1f, 1024.0f);
    camera_.cameraData.pos = camera_.pos_;
    camera_.cameraBuffer_->Copy(&camera_.cameraData);

    BufferInput bufferInput;
    for (auto& light : lights_)
        light.maxLights = lights_.size();
    if (!lights_.empty() && lightDirtyFlag_) {
        bufferInput = { sizeof(LightData) * lights_.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_.reset();
        lightDataBuffer_ = std::make_unique<Buffer>(bufferInput);
        lightDataBuffer_->Copy(lights_.data());
        meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        shadowMapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        std::vector<vk::WriteDescriptorSet> light = {
            { meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &meshRenderPipeline.lightDescriptor },
            { shadowMapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &shadowMapPipeline.lightDescriptor }
        };
        Device::GetBundle().device.updateDescriptorSets(light, nullptr);
        lightDirtyFlag_ = false;
        shadowMapDirtyFlag_ = true;
    }

    if (GetInstanceCount() > 0 && meshDirtyFlag_) {
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(meshes_.size());
        for (auto& mesh : meshes_) {
            if (mesh.GetInstanceCount() < 1)
                continue;
            bufferInput = { sizeof(MeshInstance) * mesh.GetInstanceCount(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
            mesh.meshInstanceBuffer_.reset();
            mesh.meshInstanceBuffer_ = std::make_unique<Buffer>(bufferInput);
            mesh.meshInstanceBuffer_->Copy(mesh.meshInstances_.data());

            bufferInfos.emplace_back(mesh.meshInstanceBuffer_->GetBundle().descriptorBufferInfo);
        }
        meshRenderPipeline.meshDescriptors = bufferInfos;
        shadowMapPipeline.meshDescriptors = bufferInfos;
        std::vector<vk::WriteDescriptorSet> mesh = {
            { meshRenderPipeline.descriptorSets[0], 2, 0, (uint32_t)bufferInfos.size(), vk::DescriptorType::eStorageBuffer, {}, bufferInfos.data() },
            { shadowMapPipeline.descriptorSets[0], 2, 0, (uint32_t)bufferInfos.size(), vk::DescriptorType::eStorageBuffer, {}, bufferInfos.data() }
        };
        Device::GetBundle().device.updateDescriptorSets(mesh, nullptr);
        meshDirtyFlag_ = false;
        shadowMapDirtyFlag_ = true;
    }

    shadowMapCameraData_.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1024.f);
    shadowMapCameraBuffer_->Copy(&shadowMapCameraData_);

    if (shadowMapDirtyFlag_) {
        for (int i = 0; i < lights_.size(); i++) {
            shadowMaps_[i].DrawShadowMap(commandBuffer_, i, lights_, meshes_);
        }
        shadowMapDirtyFlag_ = false;
    }

    std::vector<vk::DescriptorImageInfo> textureInfos;
    for (auto& texture : textures_) {
        textureInfos.push_back(texture.image.GetBundle().imageInfo);
    }
    std::vector<vk::DescriptorImageInfo> cubeMapInfos;
    for (auto& cubeMapInfo : meshRenderPipeline.shadowCubeMapDescriptors) {
        cubeMapInfos.push_back(cubeMapInfo);
    }
    std::vector<vk::DescriptorImageInfo> diffuseTextureInfos;
    for (auto& diffuse : diffuseTextures_) {
        diffuseTextureInfos.push_back(diffuse.image.GetBundle().imageInfo);
    }
    std::vector<vk::DescriptorImageInfo> normalTextureInfos;
    for (auto& normal : normalTextures_) {
        normalTextureInfos.push_back(normal.image.GetBundle().imageInfo);
    }
    std::vector<vk::WriteDescriptorSet> image = {
        { meshRenderPipeline.descriptorSets[1], 0, 0, (uint32_t)textureInfos.size(), vk::DescriptorType::eCombinedImageSampler, textureInfos.data() },
        { meshRenderPipeline.descriptorSets[1], 1, 0, (uint32_t)cubeMapInfos.size(), vk::DescriptorType::eCombinedImageSampler, cubeMapInfos.data() },
        { meshRenderPipeline.descriptorSets[1], 2, 0, (uint32_t)diffuseTextureInfos.size(), vk::DescriptorType::eCombinedImageSampler, diffuseTextureInfos.data() },
        { meshRenderPipeline.descriptorSets[1], 3, 0, (uint32_t)normalTextureInfos.size(), vk::DescriptorType::eCombinedImageSampler, normalTextureInfos.data() }
    };
    Device::GetBundle().device.updateDescriptorSets(image, nullptr);
}

void Scene::AddLight()
{
    lights_.emplace_back();
    shadowMaps_.emplace_back();
    shadowMaps_.back().PrepareShadowCubeMap(commandBuffer_);
    lightDirtyFlag_ = true;
}

void Scene::DeleteMesh()
{
    if (selectedMeshID_ < 0 || selectedMeshInstanceID_ < 0)
        return;
    meshes_[selectedMeshID_].meshInstances_.erase(meshes_[selectedMeshID_].meshInstances_.begin() + selectedMeshInstanceID_);

    for (int32_t i = selectedMeshInstanceID_; i < meshes_[selectedMeshID_].meshInstances_.size(); i++) {
        meshes_[selectedMeshID_].meshInstances_[i].instanceID--;
    }
    meshes_[selectedMeshID_].instanceID_--;

    selectedMeshID_ = -1;
    selectedMeshInstanceID_ = -1;
    meshDirtyFlag_ = true;
}

void Scene::DeleteLight()
{
    if (selectedLightID_ < 0)
        return;
    lights_.erase(lights_.begin() + selectedLightID_);

    selectedLightID_ = -1;
    lightDirtyFlag_ = true;
}

size_t Scene::GetInstanceCount()
{
    size_t instanceCount = 0;
    for (auto& mesh : meshes_) {
        instanceCount += mesh.GetInstanceCount();
    }
    return instanceCount;
}

void Scene::SelectByColorID(Viewport& viewport)
{
    double mouseX = 0;
    double mouseY = 0;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
    const int32_t* colorID = viewport.PickColor(mouseX, mouseY);
    selectedMeshID_ = colorID[0];
    selectedMeshInstanceID_ = colorID[1];
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

    if (fileFormat == "obj" || fileFormat == "gltf") {
        resourceType = RESOURCETYPE::MESH;
    } else if (fileFormat == "png" || fileFormat == "jpg" || fileFormat == "tga") {
        resourceType = RESOURCETYPE::TEXTURE;
    }
}
