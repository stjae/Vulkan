#include "scene.h"

Scene::Scene() : selectedMeshID_(-1), selectedMeshInstanceID_(-1), selectedLightID_(-1), meshDirtyFlag_(true), lightDirtyFlag_(true), shadowMapDirtyFlag_(true), showLightIcon_(true), envCube_(PRIMITIVE::CUBE), brdfLutSquare_(PRIMITIVE::SQUARE), saveFilePath_(""), iblExposure_(1.0f), resourceDirtyFlag_(true), envCubemapDirtyFlag_(true)
{
    vkn::Command::CreateCommandPool(commandPool_);
    vkn::Command::AllocateCommandBuffer(commandPool_, commandBuffer_);
    vkn::Command::AllocateCommandBuffer(commandPool_, Mesh::commandBuffer_);

    {
        // init camera pos
        camera_.dir_ = { 0.5, -0.3, -0.7, 0.0 };
        camera_.pos_ = { -3.0, 3.3, 8.0 };
        camera_.at_ = { -2.5, 3.0, 7.3 };
    }

    {
        // prepare textures
        vkn::Image::CreateSampler();
        albedoTextures_.reserve(1000);
        normalTextures_.reserve(1000);
        metallicTextures_.reserve(1000);
        roughnessTextures_.reserve(1000);
        shadowMaps_.reserve(1000);
        vk::DescriptorImageInfo samplerInfo(vkn::Image::repeatSampler);
        vk::WriteDescriptorSet writeDescriptorSet(meshRenderPipeline.descriptorSets[1], 0, 0, 1, vk::DescriptorType::eSampler, &samplerInfo);
        vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    {
        // prepare buffer, descriptor
        vkn::BufferInput bufferInput = { sizeof(CameraData), sizeof(CameraData), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        shadowMapCameraBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
        shadowCubemapPipeline.cameraDescriptor = shadowMapCameraBuffer_->GetBundle().descriptorBufferInfo;
        bufferInput = { sizeof(LightData), sizeof(LightData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
        meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        shadowCubemapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
    }

    {
        // init scene
        envCube_.CreateCube();
        envCube_.CreateBuffers();
        envCube_.meshInstances_.emplace_back(0, 0);
        brdfLutSquare_.CreateSquare();
        brdfLutSquare_.CreateBuffers();
        brdfLutSquare_.meshInstances_.emplace_back(0, 0);
        brdfLut_.CreateImage({ 512, 512, 1 }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::clampSampler);
        brdfLut_.CreateImageView();
        brdfLut_.CreateFramebuffer(brdfLutPipeline);
        brdfLut_.Draw(brdfLutSquare_, brdfLutPipeline, commandBuffer_);
    }

    InitHdri();
}

void Scene::AddResource(std::string& filePath)
{
    resources_.emplace_back(filePath);
    meshes_.emplace_back(meshes_.size() - 1, resources_.back().filePath);
    meshes_.back().CreateBuffers();
    meshes_.back().meshID_++;
    resources_.back().ptr = &meshes_.back();

    if (!meshes_.back().materials_.empty()) {
        LoadMaterials(filePath, meshes_.back().materials_);
        resourceDirtyFlag_ = true;
    }
}

void Scene::LoadMaterials(const std::string& modelPath, const std::vector<Material>& materials)
{
    std::vector<vk::CommandPool> commandPools;
    commandPools.resize(4);
    for (auto& commandPool : commandPools)
        vkn::Command::CreateCommandPool(commandPool);
    std::vector<vk::CommandBuffer> commandBuffers;
    commandBuffers.resize(4);

    for (auto& material : materials) {
        albedoTextures_.emplace_back();
        std::thread t0 = std::thread([&]() {
            vkn::Command::AllocateCommandBuffer(commandPools[0], commandBuffers[0]);
            albedoTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.albedo, vk::Format::eR8G8B8A8Unorm, commandBuffers[0]);
        });
        normalTextures_.emplace_back();
        std::thread t1 = std::thread([&]() {
            vkn::Command::AllocateCommandBuffer(commandPools[1], commandBuffers[1]);
            normalTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normal, vk::Format::eR8G8B8A8Unorm, commandBuffers[1]);
        });
        metallicTextures_.emplace_back();
        std::thread t2 = std::thread([&]() {
            vkn::Command::AllocateCommandBuffer(commandPools[2], commandBuffers[2]);
            metallicTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.metallic, vk::Format::eR8G8B8A8Unorm, commandBuffers[2]);
        });
        roughnessTextures_.emplace_back();
        std::thread t3 = std::thread([&]() {
            vkn::Command::AllocateCommandBuffer(commandPools[3], commandBuffers[3]);
            roughnessTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.roughness, vk::Format::eR8G8B8A8Unorm, commandBuffers[3]);
        });

        t0.join();
        t1.join();
        t2.join();
        t3.join();

        vkn::Command::Submit(commandBuffers.data(), commandBuffers.size());
    }

    for (auto& commandPool : commandPools)
        vkn::Device::GetBundle().device.destroyCommandPool(commandPool);
}

void Scene::AddMeshInstance(uint32_t id, glm::vec3 pos, glm::vec3 scale)
{
    meshes_[id].AddInstance(pos, scale);
    meshDirtyFlag_ = true;
}

void Scene::Update()
{
    HandleLightDuplication();
    HandleMeshDuplication();

    UpdateCamera();
    UpdateLight();
    UpdateMesh();
    UpdateShadowMap();
    UpdateDescriptorSet();
}

void Scene::AddLight()
{
    pointLights_.emplace_back();
    shadowMaps_.emplace_back();
    shadowMaps_.back().CreateShadowMap(commandBuffer_);
    lightDirtyFlag_ = true;
}

void Scene::AddEnvironmentMap(const std::string& hdriFilePath)
{
    hdriFilePath_ = hdriFilePath;
    envMap_ = std::make_unique<vkn::Image>();
    envMap_->InsertHDRImage(hdriFilePath, vk::Format::eR32G32B32A32Sfloat, commandBuffer_);
    envCubemap_ = std::make_unique<EnvCubemap>();
    envCubemap_->CreateEnvCubemap(512, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, envCubemapPipeline, commandBuffer_);
    envCubemap_->DrawEnvCubemap(envCube_, *envMap_, envCubemapPipeline, commandBuffer_);
    irradianceCubemap_ = std::make_unique<EnvCubemap>();
    irradianceCubemap_->CreateEnvCubemap(32, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, irradianceCubemapPipeline, commandBuffer_);
    irradianceCubemap_->DrawEnvCubemap(envCube_, *envCubemap_, irradianceCubemapPipeline, commandBuffer_);
    prefilteredCubemap_ = std::make_unique<PrefilteredCubemap>();
    prefilteredCubemap_->CreatePrefilteredCubemap(5, 128, prefilteredCubemapPipeline, commandBuffer_);
    prefilteredCubemap_->DrawPrefilteredCubemap(envCube_, *envCubemap_, prefilteredCubemapPipeline, commandBuffer_);
    envCubemapDirtyFlag_ = true;
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
    pointLights_.erase(pointLights_.begin() + selectedLightID_);

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

void Scene::SelectByColorID(int32_t meshID, int32_t instanceID)
{
    selectedMeshID_ = meshID;
    selectedMeshInstanceID_ = instanceID;
}

void Scene::HandleLightDuplication()
{
    if (selectedLightID_ > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !camera_.isControllable_) {
        AddLight();
        pointLights_.back() = pointLights_[selectedLightID_];
    }
}

void Scene::HandleMeshDuplication()
{
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
}

void Scene::UpdateCamera()
{
    camera_.Update();
    if (selectedMeshID_ > -1 && selectedMeshInstanceID_ > -1 && ImGui::IsKeyDown(ImGuiKey_F)) {
        camera_.pos_ = glm::translate(meshes_[selectedMeshID_].meshInstances_[selectedMeshInstanceID_].model, glm::vec3(0.0f, 0.0f, 2.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    camera_.cameraData.view = glm::lookAt(camera_.pos_, camera_.at_, camera_.up_);
    camera_.cameraData.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::GetBundle().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::GetBundle().swapchainImageExtent.height), 0.1f, 1024.0f);
    camera_.cameraData.pos = camera_.pos_;
    camera_.cameraBuffer_->Copy(&camera_.cameraData);
}

void Scene::UpdateLight()
{
    if (!pointLights_.empty() && lightDirtyFlag_) {
        vkn::BufferInput bufferInput = { sizeof(LightData) * pointLights_.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_.reset();
        lightDataBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
        lightDataBuffer_->Copy(pointLights_.data());
        meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        shadowCubemapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        std::vector<vk::WriteDescriptorSet> light = {
            { meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &meshRenderPipeline.lightDescriptor },
            { shadowCubemapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &shadowCubemapPipeline.lightDescriptor }
        };
        vkn::Device::GetBundle().device.updateDescriptorSets(light, nullptr);
        lightDirtyFlag_ = false;
        shadowMapDirtyFlag_ = true;
    }
}

void Scene::UpdateMesh()
{
    if (GetInstanceCount() > 0 && meshDirtyFlag_) {
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(meshes_.size());
        for (auto& mesh : meshes_) {
            if (mesh.GetInstanceCount() < 1)
                continue;
            vkn::BufferInput bufferInput = { sizeof(MeshInstance) * mesh.GetInstanceCount(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
            mesh.meshInstanceBuffer_.reset();
            mesh.meshInstanceBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
            mesh.meshInstanceBuffer_->Copy(mesh.meshInstances_.data());

            bufferInfos.emplace_back(mesh.meshInstanceBuffer_->GetBundle().descriptorBufferInfo);
        }
        meshRenderPipeline.meshDescriptors = bufferInfos;
        shadowCubemapPipeline.meshDescriptors = bufferInfos;
        std::vector<vk::WriteDescriptorSet> mesh = {
            { meshRenderPipeline.descriptorSets[0], 2, 0, (uint32_t)bufferInfos.size(), vk::DescriptorType::eStorageBuffer, {}, bufferInfos.data() },
            { shadowCubemapPipeline.descriptorSets[0], 2, 0, (uint32_t)bufferInfos.size(), vk::DescriptorType::eStorageBuffer, {}, bufferInfos.data() }
        };
        vkn::Device::GetBundle().device.updateDescriptorSets(mesh, nullptr);
        meshDirtyFlag_ = false;
        shadowMapDirtyFlag_ = true;
    }
}

void Scene::UpdateShadowMap()
{
    shadowMapCameraData_.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 1024.f);
    shadowMapCameraBuffer_->Copy(&shadowMapCameraData_);

    if (shadowMapDirtyFlag_) {
        for (int i = 0; i < pointLights_.size(); i++) {
            shadowMaps_[i].DrawShadowMap(commandBuffer_, i, pointLights_, meshes_);
        }

        UpdateShadowCubemapDescriptors();

        shadowMapDirtyFlag_ = false;
    }
}

void Scene::UpdateUniformDescriptors()
{
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(2);

    writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &meshRenderPipeline.cameraDescriptor);
    writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &meshRenderPipeline.lightDescriptor);
    writeDescriptorSets.emplace_back(shadowCubemapPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &shadowCubemapPipeline.cameraDescriptor);
    writeDescriptorSets.emplace_back(shadowCubemapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &shadowCubemapPipeline.lightDescriptor);

    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

void Scene::UpdateTextureDescriptors()
{
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    for (int i = 0; i < albedoTextures_.size(); i++) {
        writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[1], 1, i, 1, vk::DescriptorType::eSampledImage, &albedoTextures_[i].GetBundle().descriptorImageInfo);
    }
    for (int i = 0; i < normalTextures_.size(); i++) {
        writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[1], 2, i, 1, vk::DescriptorType::eSampledImage, &normalTextures_[i].GetBundle().descriptorImageInfo);
    }
    for (int i = 0; i < metallicTextures_.size(); i++) {
        writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[1], 3, i, 1, vk::DescriptorType::eSampledImage, &metallicTextures_[i].GetBundle().descriptorImageInfo);
    }
    for (int i = 0; i < roughnessTextures_.size(); i++) {
        writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[1], 4, i, 1, vk::DescriptorType::eSampledImage, &roughnessTextures_[i].GetBundle().descriptorImageInfo);
    }
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSets, nullptr);
    resourceDirtyFlag_ = false;
}

void Scene::UpdateShadowCubemapDescriptors()
{
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    std::vector<vk::DescriptorImageInfo> shadowCubeMapDescriptors;
    shadowCubeMapDescriptors.reserve(shadowMaps_.size());
    for (auto& shadowMap : shadowMaps_)
        shadowCubeMapDescriptors.emplace_back(nullptr, shadowMap.GetBundle().imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
    if (!shadowCubeMapDescriptors.empty())
        writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[1], 5, 0, shadowCubeMapDescriptors.size(), vk::DescriptorType::eSampledImage, shadowCubeMapDescriptors.data());
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

void Scene::UpdateEnvCubemapDescriptors()
{
    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.emplace_back(skyboxRenderPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &meshRenderPipeline.cameraDescriptor);
    writeDescriptorSets.emplace_back(skyboxRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &irradianceCubemap_->GetBundle().descriptorImageInfo);
    writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[2], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &irradianceCubemap_->GetBundle().descriptorImageInfo);
    writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[2], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &prefilteredCubemap_->mipmapDescriptorImageInfo);
    writeDescriptorSets.emplace_back(meshRenderPipeline.descriptorSets[2], 2, 0, 1, vk::DescriptorType::eCombinedImageSampler, &brdfLut_.GetBundle().descriptorImageInfo);
    vkn::Device::GetBundle().device.updateDescriptorSets(writeDescriptorSets, nullptr);
    envCubemapDirtyFlag_ = false;
}

void Scene::UpdateDescriptorSet()
{
    UpdateUniformDescriptors();
    if (resourceDirtyFlag_)
        UpdateTextureDescriptors();
    if (envCubemapDirtyFlag_)
        UpdateEnvCubemapDescriptors();
}

void Scene::InitScene()
{
    saveFilePath_ = "";
    hdriFilePath_ = "";
    envMap_.reset();
    envCubemap_.reset();
    irradianceCubemap_.reset();
    prefilteredCubemap_.reset();

    meshes_.clear();
    resources_.clear();
    pointLights_.clear();
    shadowMaps_.clear();
    albedoTextures_.clear();
    normalTextures_.clear();
    metallicTextures_.clear();
    roughnessTextures_.clear();

    InitHdri();
}

void Scene::InitHdri()
{
    envMap_ = std::make_unique<vkn::Image>();
    envMap_->InsertDummyImage(commandBuffer_, { 128, 128, 128, 255 });
    vkn::Command::Submit(&commandBuffer_, 1);
    envCubemap_ = std::make_unique<EnvCubemap>();
    envCubemap_->CreateEnvCubemap(512, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, envCubemapPipeline, commandBuffer_);
    envCubemap_->DrawEnvCubemap(envCube_, *envMap_, envCubemapPipeline, commandBuffer_);
    irradianceCubemap_ = std::make_unique<EnvCubemap>();
    irradianceCubemap_->CreateEnvCubemap(32, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, irradianceCubemapPipeline, commandBuffer_);
    irradianceCubemap_->DrawEnvCubemap(envCube_, *envCubemap_, irradianceCubemapPipeline, commandBuffer_);
    prefilteredCubemap_ = std::make_unique<PrefilteredCubemap>();
    prefilteredCubemap_->CreatePrefilteredCubemap(5, 128, prefilteredCubemapPipeline, commandBuffer_);
    prefilteredCubemap_->DrawPrefilteredCubemap(envCube_, *envCubemap_, prefilteredCubemapPipeline, commandBuffer_);
    envCubemapDirtyFlag_ = true;
}

Scene::~Scene()
{
    vkn::Device::GetBundle().device.freeCommandBuffers(commandPool_, commandBuffer_);
    vkn::Device::GetBundle().device.destroyCommandPool(commandPool_);
}
