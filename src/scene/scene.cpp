#include "scene.h"

Scene::Scene() : m_selectedMeshID(-1), m_selectedMeshInstanceID(-1), m_selectedLightID(-1), m_meshDirtyFlag(true), m_lightDirtyFlag(true), m_shadowShadowCubemapDirtyFlag(true), m_showLightIcon(true), m_envCube(), m_brdfLutSquare(), m_saveFilePath(), m_iblExposure(1.0f), m_resourceDirtyFlag(true), m_envCubemapDirtyFlag(true), m_isPlaying(false)
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffers);

    {
        // init camera pos
        m_camera.dir_ = { 0.5, -0.3, -0.7 };
        m_camera.pos_ = { -3.0, 3.3, 8.0 };
        m_camera.at_ = { -2.5, 3.0, 7.3 };

        meshRenderPipeline.m_cameraDescriptor = m_camera.cameraBuffer_->Get().descriptorBufferInfo;
        meshRenderPipeline.UpdateCameraDescriptor();
        skyboxRenderPipeline.m_cameraDescriptor = m_camera.cameraBuffer_->Get().descriptorBufferInfo;
        skyboxRenderPipeline.UpdateCameraDescriptor();
        lineRenderPipeline.m_cameraDescriptor = m_camera.cameraBuffer_->Get().descriptorBufferInfo;
        lineRenderPipeline.UpdateCameraDescriptor();
    }

    // TODO:
    // m_meshes.reserve(10);

    {
        // prepare textures
        vkn::Image::CreateSampler();
        m_shadowMap.CreateShadowMap(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        meshRenderPipeline.m_shadowMapImageDescriptor = { vkn::Image::s_repeatSampler, m_shadowMap.Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal };
        meshRenderPipeline.UpdateShadowMapDescriptor();
        vk::DescriptorImageInfo samplerInfo(vkn::Image::s_repeatSampler);
        vk::WriteDescriptorSet writeDescriptorSet(meshRenderPipeline.m_descriptorSets[1], 0, 0, 1, vk::DescriptorType::eSampler, &samplerInfo);
        vkn::Device::Get().device.updateDescriptorSets(writeDescriptorSet, nullptr);
    }

    {
        // prepare buffer, descriptor
        vkn::BufferInfo bufferInput = { sizeof(glm::mat4), sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_shadowMapViewSpaceProjBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        shadowMapPipeline.m_shadowMapSpaceViewProjDescriptor = m_shadowMapViewSpaceProjBuffer->Get().descriptorBufferInfo;
        meshRenderPipeline.m_shadowMapSpaceViewProjDescriptor = m_shadowMapViewSpaceProjBuffer->Get().descriptorBufferInfo;
        m_shadowCubemapProjBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        shadowCubemapPipeline.m_projDescriptor = m_shadowCubemapProjBuffer->Get().descriptorBufferInfo;
        shadowCubemapPipeline.UpdateProjDescriptor();
        m_shadowCubemapProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 1024.f);
        m_shadowCubemapProjBuffer->Copy(&m_shadowCubemapProj);
        bufferInput = { sizeof(DirLightUBO), sizeof(DirLightUBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_dirLightDataBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        meshRenderPipeline.m_dirLightDescriptor = m_dirLightDataBuffer->Get().descriptorBufferInfo;
        meshRenderPipeline.UpdateDirLightDescriptor();
        bufferInput = { sizeof(PointLightUBO), sizeof(PointLightUBO), vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_pointLightDataBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        meshRenderPipeline.m_pointLightDescriptor = m_pointLightDataBuffer->Get().descriptorBufferInfo;
        shadowCubemapPipeline.m_pointLightDescriptor = m_pointLightDataBuffer->Get().descriptorBufferInfo;
    }

    {
        // init scene
        m_envCube.CreateCube();
        m_envCube.CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        m_envCube.meshInstanceUBOs_.emplace_back(0, 0);
        m_brdfLutSquare.CreateSquare();
        m_brdfLutSquare.CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        m_brdfLutSquare.meshInstanceUBOs_.emplace_back(0, 0);
        m_brdfLut.CreateImage({ 512, 512, 1 }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);
        m_brdfLut.CreateImageView();
        m_brdfLut.CreateFramebuffer(brdfLutPipeline);
        m_brdfLut.Draw(m_brdfLutSquare, brdfLutPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        meshRenderPipeline.m_brdfLutDescriptor = m_brdfLut.Get().descriptorImageInfo;
        meshRenderPipeline.UpdateBrdfLutDescriptor();

        m_physics.InitPhysics();
    }

    // m_meshes.emplace_back(0);
    // m_meshes.back().CreateCube();
    // m_meshes.back().CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    // m_meshes.back().AddInstance();

    InitHdri();
}

void Scene::AddResource(std::string& filePath)
{
    m_resources.emplace_back(filePath);
    m_meshes.emplace_back(m_meshes.size(), m_resources.back().filePath);
    m_meshes.back().CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_resources.back().ptr = &m_meshes.back();

    if (!m_meshes.back().materials_.empty()) {
        LoadMaterials(filePath, m_meshes.back().materials_);
        m_resourceDirtyFlag = true;
    }
}

void Scene::LoadMaterials(const std::string& modelPath, const std::vector<MaterialFilePath>& materials)
{
    // TODO: Fix
    // std::array<vk::CommandBuffer, 4> commandBuffers;
    // vkn::Command::AllocateCommandBuffer(m_commandPool, commandBuffers);
    //
    // for (auto& material : materials) {
    //     m_albedoTextures.emplace_back();
    //     m_albedoTextures.back() = std::make_unique<vkn::Image>();
    //     std::thread t0 = std::thread([&]() {
    //         m_albedoTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.albedo, vk::Format::eR8G8B8A8Unorm, commandBuffers[0]);
    //     });
    //     m_normalTextures.emplace_back();
    //     m_normalTextures.back() = std::make_unique<vkn::Image>();
    //     std::thread t1 = std::thread([&]() {
    //         m_normalTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normal, vk::Format::eR8G8B8A8Unorm, commandBuffers[1]);
    //     });
    //     m_metallicTextures.emplace_back();
    //     m_metallicTextures.back() = std::make_unique<vkn::Image>();
    //     std::thread t2 = std::thread([&]() {
    //         m_metallicTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.metallic, vk::Format::eR8G8B8A8Unorm, commandBuffers[2]);
    //     });
    //     m_roughnessTextures.emplace_back();
    //     m_roughnessTextures.back() = std::make_unique<vkn::Image>();
    //     std::thread t3 = std::thread([&]() {
    //         m_roughnessTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.roughness, vk::Format::eR8G8B8A8Unorm, commandBuffers[3]);
    //     });
    //
    //     t0.join();
    //     t1.join();
    //     t2.join();
    //     t3.join();
    //
    //     vkn::Command::Submit(commandBuffers);
    // }
    //
    // for (auto& commandPool : commandPools)
    //     vkn::Device::Get().device.destroyCommandPool(commandPool);
}

void Scene::AddMeshInstance(uint32_t id, glm::vec3 pos, glm::vec3 scale)
{
    m_meshes[id].AddInstance(pos, scale);
    m_meshDirtyFlag = true;
}

void Scene::Update()
{
    HandlePointLightDuplication();
    HandleMeshDuplication();

    UpdateCamera();
    UpdatePointLight();
    UpdateMesh();
    UpdateShadowMap();
    UpdateShadowCubemaps();
    UpdateDescriptorSet();
}

void Scene::AddLight()
{
    m_pointLights.emplace_back();
    m_shadowCubemaps.emplace_back();
    m_shadowCubemaps.back() = std::make_unique<ShadowCubemap>();
    m_shadowCubemaps.back()->CreateShadowMap(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_lightDirtyFlag = true;
}

void Scene::AddEnvironmentMap(const std::string& hdriFilePath)
{
    m_hdriFilePath = hdriFilePath;
    m_envMap = std::make_unique<vkn::Image>();
    m_envMap->InsertHDRImage(hdriFilePath, vk::Format::eR32G32B32A32Sfloat, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_envCubemap = std::make_unique<EnvCubemap>();
    m_envCubemap->CreateEnvCubemap(512, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, envCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_envCubemap->DrawEnvCubemap(m_envCube, *m_envMap, envCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_irradianceCubemap = std::make_unique<EnvCubemap>();
    m_irradianceCubemap->CreateEnvCubemap(32, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, irradianceCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_irradianceCubemap->DrawEnvCubemap(m_envCube, *m_envCubemap, irradianceCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_prefilteredCubemap = std::make_unique<PrefilteredCubemap>();
    m_prefilteredCubemap->CreatePrefilteredCubemap(5, 128, prefilteredCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_prefilteredCubemap->DrawPrefilteredCubemap(m_envCube, *m_envCubemap, prefilteredCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_envCubemapDirtyFlag = true;
}

void Scene::DeleteMeshInstance()
{
    if (m_selectedMeshID < 0 || m_selectedMeshInstanceID < 0)
        return;
    m_meshes[m_selectedMeshID].meshInstanceUBOs_.erase(m_meshes[m_selectedMeshID].meshInstanceUBOs_.begin() + m_selectedMeshInstanceID);

    for (int32_t i = m_selectedMeshInstanceID; i < m_meshes[m_selectedMeshID].meshInstanceUBOs_.size(); i++) {
        m_meshes[m_selectedMeshID].meshInstanceUBOs_[i].instanceID--;
    }

    m_selectedMeshID = -1;
    m_selectedMeshInstanceID = -1;
    m_meshDirtyFlag = true;
}

void Scene::DeletePointLight()
{
    if (m_selectedLightID < 0)
        return;
    m_pointLights.erase(m_pointLights.begin() + m_selectedLightID);

    m_selectedLightID = -1;
    m_lightDirtyFlag = true;
}

size_t Scene::GetInstanceCount()
{
    size_t instanceCount = 0;
    for (auto& mesh : m_meshes) {
        instanceCount += mesh.GetInstanceCount();
    }
    return instanceCount;
}

void Scene::SelectByColorID(int32_t meshID, int32_t instanceID)
{
    m_selectedMeshID = meshID;
    m_selectedMeshInstanceID = instanceID;
}

void Scene::HandlePointLightDuplication()
{
    if (m_selectedLightID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_camera.isControllable_) {
        AddLight();
        m_pointLights.back() = m_pointLights[m_selectedLightID];
    }
}

void Scene::HandleMeshDuplication()
{
    if (m_selectedMeshID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_camera.isControllable_) {
        AddMeshInstance(m_selectedMeshID);
        auto& newMeshInstanceUBO = m_meshes[m_selectedMeshID].meshInstanceUBOs_.back();
        int newMeshInstanceID = newMeshInstanceUBO.instanceID;
        // copy data of source instance
        auto& srcMeshInstanceUBO = m_meshes[m_selectedMeshID].meshInstanceUBOs_[m_selectedMeshInstanceID];
        newMeshInstanceUBO = srcMeshInstanceUBO;
        // except for id
        newMeshInstanceUBO.instanceID = newMeshInstanceID;
        // select new instance
        m_selectedMeshInstanceID = newMeshInstanceID;

        if (srcMeshInstanceUBO.physicsInfo) {
            // m_physics.AddRigidBodies(m_meshes[m_selectedMeshID], newMeshInstanceUBO, *srcMeshInstanceUBO.physicsInfo);
        }
    }
}

void Scene::UpdateCamera()
{
    m_camera.Update();
    if (m_selectedMeshID > -1 && m_selectedMeshInstanceID > -1 && ImGui::IsKeyDown(ImGuiKey_F)) {
        m_camera.pos_ = glm::translate(m_meshes[m_selectedMeshID].meshInstanceUBOs_[m_selectedMeshInstanceID].model, glm::vec3(0.0f, 0.0f, 2.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    m_camera.cameraUBO_.view = glm::lookAt(m_camera.pos_, m_camera.at_, m_camera.up_);
    m_camera.cameraUBO_.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), 0.1f, 1024.0f);
    m_camera.cameraUBO_.pos = m_camera.pos_;
    m_camera.cameraBuffer_->Copy(&m_camera.cameraUBO_);
}

void Scene::UpdatePointLight()
{
    if (!m_pointLights.empty() && m_lightDirtyFlag) {
        vkn::BufferInfo bufferInput = { sizeof(PointLightUBO) * m_pointLights.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_pointLightDataBuffer.reset();
        m_pointLightDataBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        m_pointLightDataBuffer->Copy(m_pointLights.data());
        meshRenderPipeline.m_pointLightDescriptor = m_pointLightDataBuffer->Get().descriptorBufferInfo;
        shadowCubemapPipeline.m_pointLightDescriptor = m_pointLightDataBuffer->Get().descriptorBufferInfo;
        meshRenderPipeline.UpdatePointLightDescriptor();
        shadowCubemapPipeline.UpdatePointLightDescriptor();

        m_lightDirtyFlag = false;
        m_shadowShadowCubemapDirtyFlag = true;
    }
}

void Scene::UpdateShadowMap()
{
    m_dirLightUBO.dir = glm::normalize(m_dirLightPos);
    m_dirLightDataBuffer->Copy(&m_dirLightUBO);

    glm::mat4 lightProjection = glm::ortho(-1.0f * m_dirLightSize, m_dirLightSize, -1.0f * m_dirLightSize, m_dirLightSize, m_dirLightNearPlane, m_dirLightFarPlane);
    glm::mat4 lightView = glm::lookAt(m_dirLightPos, glm::vec3(0.0f), glm::vec3(0.0, 0.0f, 1.0f));
    m_shadowMapViewProj = lightProjection * lightView;
    m_shadowMapViewSpaceProjBuffer->Copy(&m_shadowMapViewProj);
    m_shadowMap.DrawShadowMap(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], m_meshes);
}

void Scene::UpdateShadowCubemaps()
{
    if (m_shadowShadowCubemapDirtyFlag) {
        for (int i = 0; i < m_pointLights.size(); i++) {
            m_shadowCubemaps[i]->DrawShadowMap(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], i, m_pointLights, m_meshes);
        }
        meshRenderPipeline.m_shadowCubemapDescriptors.clear();
        for (auto& shadowCubemap : m_shadowCubemaps) {
            meshRenderPipeline.m_shadowCubemapDescriptors.emplace_back(nullptr, shadowCubemap->Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        }
        meshRenderPipeline.UpdateShadowCubemapDescriptors();
        m_shadowShadowCubemapDirtyFlag = false;
    }
}

void Scene::UpdateMesh()
{
    if (GetInstanceCount() > 0 && m_meshDirtyFlag) {
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(m_meshes.size());
        for (auto& mesh : m_meshes) {
            if (mesh.GetInstanceCount() < 1)
                continue;
            vkn::BufferInfo bufferInput = { sizeof(MeshInstanceUBO) * mesh.GetInstanceCount(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
            mesh.meshInstanceBuffer_.reset();
            mesh.meshInstanceBuffer_ = std::make_unique<vkn::Buffer>(bufferInput);
            mesh.meshInstanceBuffer_->Copy(mesh.meshInstanceUBOs_.data());

            bufferInfos.emplace_back(mesh.meshInstanceBuffer_->Get().descriptorBufferInfo);
        }
        meshRenderPipeline.m_meshDescriptors = bufferInfos;
        shadowMapPipeline.m_meshDescriptors = bufferInfos;
        shadowCubemapPipeline.m_meshDescriptors = bufferInfos;
        lineRenderPipeline.m_meshDescriptors = bufferInfos;
        meshRenderPipeline.UpdateMeshDescriptors();
        shadowMapPipeline.UpdateMeshDescriptors();
        shadowCubemapPipeline.UpdateMeshDescriptors();
        lineRenderPipeline.UpdateMeshDescriptors();
        m_meshDirtyFlag = false;
        m_shadowShadowCubemapDirtyFlag = true;
    }
}

void Scene::UpdateUniformDescriptors()
{
    meshRenderPipeline.UpdateShadowMapSpaceViewProjDescriptor();
    shadowMapPipeline.UpdateShadowMapSpaceViewProjDescriptor();
}

void Scene::UpdateTextureDescriptors()
{
    meshRenderPipeline.m_albeodoTextureDescriptors.clear();
    for (const auto& albedoTexture : m_albedoTextures) {
        meshRenderPipeline.m_albeodoTextureDescriptors.emplace_back(albedoTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateAlbedoTextureWriteDescriptors();
    meshRenderPipeline.m_normalTextureDescriptors.clear();
    for (const auto& normalTexture : m_normalTextures) {
        meshRenderPipeline.m_normalTextureDescriptors.emplace_back(normalTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateNormalTextureWriteDescriptors();
    meshRenderPipeline.m_metallicTextureDescriptors.clear();
    for (const auto& metallicTexture : m_metallicTextures) {
        meshRenderPipeline.m_metallicTextureDescriptors.emplace_back(metallicTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateMetallicTextureWriteDescriptors();
    meshRenderPipeline.m_roughnessTextureDescriptors.clear();
    for (const auto& roughnessTexture : m_roughnessTextures) {
        meshRenderPipeline.m_roughnessTextureDescriptors.emplace_back(roughnessTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateRoughnessTextureWriteDescriptors();
    m_resourceDirtyFlag = false;
}

void Scene::UpdateEnvCubemapDescriptors()
{
    skyboxRenderPipeline.m_irradianceCubemapDescriptor = m_irradianceCubemap->Get().descriptorImageInfo;
    skyboxRenderPipeline.UpdateIrradianceCubemapDescriptor();
    meshRenderPipeline.m_irradianceCubemapDescriptor = m_irradianceCubemap->Get().descriptorImageInfo;
    meshRenderPipeline.UpdateIrraianceCubemapDescriptor();
    meshRenderPipeline.m_prefilteredCubemapDescriptor = m_prefilteredCubemap->mipmapDescriptorImageInfo;
    meshRenderPipeline.UpdatePrefilteredCubemapDescriptor();
    m_envCubemapDirtyFlag = false;
}

void Scene::UpdateDescriptorSet()
{
    UpdateUniformDescriptors();
    if (m_resourceDirtyFlag)
        UpdateTextureDescriptors();
    if (m_envCubemapDirtyFlag)
        UpdateEnvCubemapDescriptors();
}

void Scene::InitScene()
{
    m_saveFilePath.clear();
    m_hdriFilePath.clear();
    m_envMap.reset();
    m_envCubemap.reset();
    m_irradianceCubemap.reset();
    m_prefilteredCubemap.reset();

    m_meshes.clear();
    m_resources.clear();
    m_pointLights.clear();
    m_shadowCubemaps.clear();
    m_albedoTextures.clear();
    m_normalTextures.clear();
    m_metallicTextures.clear();
    m_roughnessTextures.clear();

    InitHdri();
}

void Scene::InitHdri()
{
    m_hdriFilePath.clear();
    m_envMap = std::make_unique<vkn::Image>();
    m_envMap->InsertDummyImage(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], { 128, 128, 128, 255 });
    vkn::Command::Submit(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_envCubemap = std::make_unique<EnvCubemap>();
    m_envCubemap->CreateEnvCubemap(512, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, envCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_envCubemap->DrawEnvCubemap(m_envCube, *m_envMap, envCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_irradianceCubemap = std::make_unique<EnvCubemap>();
    m_irradianceCubemap->CreateEnvCubemap(32, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, irradianceCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_irradianceCubemap->DrawEnvCubemap(m_envCube, *m_envCubemap, irradianceCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_prefilteredCubemap = std::make_unique<PrefilteredCubemap>();
    m_prefilteredCubemap->CreatePrefilteredCubemap(5, 128, prefilteredCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_prefilteredCubemap->DrawPrefilteredCubemap(m_envCube, *m_envCubemap, prefilteredCubemapPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_iblExposure = 1.0f;
    m_envCubemapDirtyFlag = true;
}

void Scene::Play()
{
    if (!m_isPlaying) {
        return;
    }
    m_physics.Simulate(m_meshes);
    m_meshDirtyFlag = true;
}

void Scene::Stop()
{
    m_physics.Stop(m_meshes);
    m_meshDirtyFlag = true;
}

Scene::~Scene()
{
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
}
