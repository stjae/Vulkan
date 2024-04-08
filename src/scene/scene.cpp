#include "scene.h"

Scene::Scene() : selectedMeshID_(-1), selectedMeshInstanceID_(-1), selectedLightID_(-1), meshDirtyFlag_(true), lightDirtyFlag_(true), shadowMapDirtyFlag_(true), showLightIcon_(true)
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
        shadowMaps_.emplace_back();
        shadowMaps_.back().CreateShadowMap(commandBuffer_);
    }

    {
        // prepare buffer, descriptor
        vkn::BufferInput bufferInput = { sizeof(CameraData), sizeof(CameraData), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        shadowMapCameraBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
        vkn::shadowMapPipeline.cameraDescriptor = shadowMapCameraBuffer_->GetBundle().descriptorBufferInfo;
        bufferInput = { sizeof(LightData), sizeof(LightData), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
        vkn::meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        vkn::shadowMapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
    }

    {
        // init scene
        meshes_.emplace_back(MESHTYPE::SQUARE);
        meshes_.back().CreateSquare();
        meshes_.back().CreateBuffers();
        meshes_.emplace_back(MESHTYPE::CUBE);
        meshes_.back().CreateCube();
        meshes_.back().CreateBuffers();
        meshes_.emplace_back(MESHTYPE::SPHERE);
        meshes_.back().CreateSphere();
        meshes_.back().CreateBuffers();
        envCube_.CreateCube();
        envCube_.CreateBuffers();
        envCube_.meshInstances_.emplace_back(0, 0);

        // AddMeshInstance(MESHTYPE::CUBE, glm::vec3(0.0f, -4.0f, 0.0f), glm::vec3(10.0f, 0.1f, 10.0f));
        // AddMeshInstance(MESHTYPE::SPHERE, glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.3f));
        // lights_.emplace_back(glm::vec3(0.0f, 1.2f, 0.5f));
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
        // case RESOURCETYPE::TEXTURE:
        //     textureArrays_.emplace_back();
        //     CreateTexture(filePath, textureArrays_.back());
        //     textureArrays_.back().descriptorSet = ImGui_ImplVulkan_AddTexture(vkn::Image::repeatSampler, textureArrays_.back().image.GetBundle().imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //     resources_.back().resource = &textureArrays_.back();
        //     break;
        // }
    }
}

void Scene::LoadMaterials(const std::string& modelPath, const std::vector<Material>& materials)
{
    for (auto& material : materials) {
        albedoTextures_.emplace_back();
        albedoTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.albedo, vk::Format::eR8G8B8A8Srgb, commandBuffer_);
        normalTextures_.emplace_back();
        normalTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normal, vk::Format::eR8G8B8A8Unorm, commandBuffer_);
        metallicTextures_.emplace_back();
        metallicTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.metallic, vk::Format::eR8G8B8A8Srgb, commandBuffer_);
        roughnessTextures_.emplace_back();
        roughnessTextures_.back().InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.roughness, vk::Format::eR8G8B8A8Srgb, commandBuffer_);
    }
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
    lights_.emplace_back();
    shadowMaps_.emplace_back();
    shadowMaps_.back().CreateShadowMap(commandBuffer_);
    lightDirtyFlag_ = true;
}

void Scene::AddEnvironmentMap()
{
    std::string hdriPath = LaunchNfd({ "HDRI", "hdr" });
    if (!hdriPath.empty()) {
        envMap_ = std::make_unique<vkn::Image>();
        envMap_->InsertHDRImage(hdriPath, vk::Format::eR32G32B32A32Sfloat, commandBuffer_);
        envCubemap_ = std::make_unique<EnvCubemap>();
        envCubemap_->CreateEnvCubemap(512, vkn::envTexPipeline, commandBuffer_);
        envCubemap_->DrawEnvCubemap(envCube_, *envMap_, vkn::envTexPipeline, commandBuffer_);
        irradianceCubemap_ = std::make_unique<EnvCubemap>();
        irradianceCubemap_->CreateEnvCubemap(32, vkn::irradianceCubemapPipeline, commandBuffer_);
        irradianceCubemap_->DrawEnvCubemap(envCube_, *envCubemap_, vkn::irradianceCubemapPipeline, commandBuffer_);
    }
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

void Scene::SelectByColorID(int32_t meshID, int32_t instanceID)
{
    selectedMeshID_ = meshID;
    selectedMeshInstanceID_ = instanceID;
}

void Scene::HandleLightDuplication()
{
    if (selectedLightID_ > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !camera_.isControllable_) {
        AddLight();
        lights_.back() = lights_[selectedLightID_];
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
    if (!lights_.empty() && lightDirtyFlag_) {
        vkn::BufferInput bufferInput = { sizeof(LightData) * lights_.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        lightDataBuffer_.reset();
        lightDataBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
        lightDataBuffer_->Copy(lights_.data());
        vkn::meshRenderPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        vkn::shadowMapPipeline.lightDescriptor = lightDataBuffer_->GetBundle().descriptorBufferInfo;
        std::vector<vk::WriteDescriptorSet> light = {
            { vkn::meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &vkn::meshRenderPipeline.lightDescriptor },
            { vkn::shadowMapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &vkn::shadowMapPipeline.lightDescriptor }
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
        vkn::meshRenderPipeline.meshDescriptors = bufferInfos;
        vkn::shadowMapPipeline.meshDescriptors = bufferInfos;
        std::vector<vk::WriteDescriptorSet> mesh = {
            { vkn::meshRenderPipeline.descriptorSets[0], 2, 0, (uint32_t)bufferInfos.size(), vk::DescriptorType::eStorageBuffer, {}, bufferInfos.data() },
            { vkn::shadowMapPipeline.descriptorSets[0], 2, 0, (uint32_t)bufferInfos.size(), vk::DescriptorType::eStorageBuffer, {}, bufferInfos.data() }
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
        for (int i = 0; i < lights_.size(); i++) {
            shadowMaps_[i].DrawShadowMap(commandBuffer_, i, lights_, meshes_);
        }
        shadowMapDirtyFlag_ = false;
    }
}

void Scene::UpdateDescriptorSet()
{
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &vkn::meshRenderPipeline.cameraDescriptor);
    descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &vkn::meshRenderPipeline.lightDescriptor);

    vk::DescriptorImageInfo samplerInfo(vkn::Image::repeatSampler);
    descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 0, 0, 1, vk::DescriptorType::eSampler, &samplerInfo);

    for (int i = 0; i < albedoTextures_.size(); i++) {
        descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 1, i, 1, vk::DescriptorType::eSampledImage, &albedoTextures_[i].GetBundle().descriptorImageInfo);
    }

    for (int i = 0; i < normalTextures_.size(); i++) {
        descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 2, i, 1, vk::DescriptorType::eSampledImage, &normalTextures_[i].GetBundle().descriptorImageInfo);
    }

    for (int i = 0; i < metallicTextures_.size(); i++) {
        descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 3, i, 1, vk::DescriptorType::eSampledImage, &metallicTextures_[i].GetBundle().descriptorImageInfo);
    }

    for (int i = 0; i < roughnessTextures_.size(); i++) {
        descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 4, i, 1, vk::DescriptorType::eSampledImage, &roughnessTextures_[i].GetBundle().descriptorImageInfo);
    }

    for (int i = 0; i < shadowMaps_.size(); i++) {
        descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 5, i, 1, vk::DescriptorType::eSampledImage, &vkn::meshRenderPipeline.shadowCubeMapDescriptors[i]);
    }

    descriptorWrites.emplace_back(vkn::shadowMapPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &vkn::shadowMapPipeline.cameraDescriptor);
    descriptorWrites.emplace_back(vkn::shadowMapPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &vkn::shadowMapPipeline.lightDescriptor);

    if (envCubemap_ != nullptr) {
        descriptorWrites.emplace_back(vkn::skyboxRenderPipeline.descriptorSets[0], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &vkn::meshRenderPipeline.cameraDescriptor);
        descriptorWrites.emplace_back(vkn::skyboxRenderPipeline.descriptorSets[0], 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &irradianceCubemap_->GetBundle().descriptorImageInfo);
        descriptorWrites.emplace_back(vkn::meshRenderPipeline.descriptorSets[1], 6, 0, 1, vk::DescriptorType::eCombinedImageSampler, &irradianceCubemap_->GetBundle().descriptorImageInfo);
    }

    vkn::Device::GetBundle().device.updateDescriptorSets(descriptorWrites, nullptr);
}

Scene::~Scene()
{
    vkn::Device::GetBundle().device.freeCommandBuffers(commandPool_, commandBuffer_);
    vkn::Device::GetBundle().device.destroyCommandPool(commandPool_);
}

Resource::Resource(std::string& path) : resource(nullptr)
{
    // TODO:
    this->filePath = path;
    this->fileName = path.substr(path.find_last_of("/\\") + 1, path.rfind('.') - path.find_last_of("/\\") - 1);
    this->fileFormat = path.substr(path.rfind('.') + 1, path.size());

    if (fileFormat == "obj" || fileFormat == "gltf") {
        resourceType = RESOURCETYPE::MESH;
    }
    // else if (fileFormat == "png" || fileFormat == "jpg" || fileFormat == "tga") {
    //     resourceType = RESOURCETYPE::TEXTURE;
    // }
}
