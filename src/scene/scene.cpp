#include "scene.h"

Scene::Scene() : m_selectedMeshID(-1), m_selectedMeshInstanceID(-1), m_selectedLightID(-1), m_meshDirtyFlag(true), m_lightDirtyFlag(true), m_shadowShadowCubemapDirtyFlag(true), m_showLightIcon(true), m_envCube(), m_brdfLutSquare(), m_sceneFilePath(), m_iblExposure(1.0f), m_resourceDirtyFlag(true), m_envCubemapDirtyFlag(true), m_isPlaying(false)
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::CreateCommandPool(ShadowCubemap::s_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffers);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_cameraCommandBuffers);
    vkn::Command::AllocateCommandBuffer(ShadowCubemap::s_commandPool, ShadowCubemap::m_commandBuffers);

    for (int i = 0; i < 4; i++) {
        vkn::Command::CreateCommandPool(m_imageLoadCommandPools[i]);
        vkn::Command::AllocateCommandBuffer(m_imageLoadCommandPools[i], m_imageLoadCommandBuffers[i]);
    }

    vkn::BufferInfo bInput = { sizeof(CameraUBO), sizeof(CameraUBO), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_cameraStagingBuffer = std::make_unique<vkn::Buffer>(bInput);
    {
        // init camera pos
        m_camera.m_dir = { 0.5, -0.3, -0.7 };
        m_camera.m_pos = { -3.0, 3.3, 8.0 };
        m_camera.m_at = { -2.5, 3.0, 7.3 };

        meshRenderPipeline.m_cameraDescriptor = m_camera.m_cameraBuffer->Get().descriptorBufferInfo;
        meshRenderPipeline.UpdateCameraDescriptor();
        skyboxRenderPipeline.m_cameraDescriptor = m_camera.m_cameraBuffer->Get().descriptorBufferInfo;
        skyboxRenderPipeline.UpdateCameraDescriptor();
        lineRenderPipeline.m_cameraDescriptor = m_camera.m_cameraBuffer->Get().descriptorBufferInfo;
        lineRenderPipeline.UpdateCameraDescriptor();
    }

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
        m_envCube.m_meshInstanceUBOs.emplace_back(0, 0);
        m_brdfLutSquare.CreateSquare();
        m_brdfLutSquare.CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        m_brdfLutSquare.m_meshInstanceUBOs.emplace_back(0, 0);
        m_brdfLut.CreateImage({ 512, 512, 1 }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);
        m_brdfLut.CreateImageView();
        m_brdfLut.CreateFramebuffer(brdfLutPipeline);
        m_brdfLut.Draw(m_brdfLutSquare, brdfLutPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        meshRenderPipeline.m_brdfLutDescriptor = m_brdfLut.Get().descriptorImageInfo;
        meshRenderPipeline.UpdateBrdfLutDescriptor();

        m_physics.InitPhysics();
    }

    InitHdri();
}

void Scene::AddResource(std::string& filePath)
{
    m_resources.emplace_back(filePath);
    m_meshes.emplace_back(m_meshes.size(), m_resources.back().filePath);
    m_meshes.back().CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_resources.back().ptr = &m_meshes.back();

    if (!m_meshes.back().m_materials.empty()) {
        LoadMaterials(filePath, m_meshes.back().m_materials);
        m_resourceDirtyFlag = true;
    }
}

void Scene::LoadMaterials(const std::string& modelPath, const std::vector<MaterialFilePath>& materials)
{
    for (int i = 0; i < 4; i++)
        vkn::Command::Begin(m_imageLoadCommandBuffers[i]);

    std::thread t0 = std::thread([&]() {
        for (auto& material : materials) {
            m_albedoTextures.emplace_back();
            m_albedoTextures.back() = std::make_unique<vkn::Image>();
            m_albedoTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.albedo, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[0]);
        }
    });
    std::thread t1 = std::thread([&]() {
        for (auto& material : materials) {
            m_normalTextures.emplace_back();
            m_normalTextures.back() = std::make_unique<vkn::Image>();
            m_normalTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normal, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[1]);
        }
    });
    std::thread t2 = std::thread([&]() {
        for (auto& material : materials) {
            m_metallicTextures.emplace_back();
            m_metallicTextures.back() = std::make_unique<vkn::Image>();
            m_metallicTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.metallic, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[2]);
        }
    });
    std::thread t3 = std::thread([&]() {
        for (auto& material : materials) {
            m_roughnessTextures.emplace_back();
            m_roughnessTextures.back() = std::make_unique<vkn::Image>();
            m_roughnessTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.roughness, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[3]);
        }
    });

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    for (int i = 0; i < 4; i++)
        m_imageLoadCommandBuffers[i].end();

    vkn::Command::SubmitAndWait(m_imageLoadCommandBuffers.size(), m_imageLoadCommandBuffers.data());
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
    m_meshes[m_selectedMeshID].m_meshInstanceUBOs.erase(m_meshes[m_selectedMeshID].m_meshInstanceUBOs.begin() + m_selectedMeshInstanceID);

    for (int32_t i = m_selectedMeshInstanceID; i < m_meshes[m_selectedMeshID].m_meshInstanceUBOs.size(); i++) {
        m_meshes[m_selectedMeshID].m_meshInstanceUBOs[i].instanceID--;
    }

    Unselect();
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
    if (m_selectedLightID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_camera.m_isControllable) {
        AddLight();
        m_pointLights.back() = m_pointLights[m_selectedLightID];
    }
}

void Scene::HandleMeshDuplication()
{
    if (m_selectedMeshID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_camera.m_isControllable) {
        AddMeshInstance(m_selectedMeshID);
        auto& newMeshInstanceUBO = m_meshes[m_selectedMeshID].m_meshInstanceUBOs.back();
        int newMeshInstanceID = newMeshInstanceUBO.instanceID;
        // copy data of source instance
        auto& srcMeshInstanceUBO = m_meshes[m_selectedMeshID].m_meshInstanceUBOs[m_selectedMeshInstanceID];
        newMeshInstanceUBO = srcMeshInstanceUBO;
        // except for id
        newMeshInstanceUBO.instanceID = newMeshInstanceID;
        // select new instance
        m_selectedMeshInstanceID = newMeshInstanceID;
    }
}

void Scene::UpdateCamera()
{
    m_camera.Update();
    if (m_selectedMeshID > -1 && m_selectedMeshInstanceID > -1 && ImGui::IsKeyDown(ImGuiKey_F)) {
        m_camera.m_pos = glm::translate(m_meshes[m_selectedMeshID].m_meshInstanceUBOs[m_selectedMeshInstanceID].model, glm::vec3(0.0f, 0.0f, 2.0f)) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    m_camera.m_cameraUBO.view = glm::lookAt(m_camera.m_pos, m_camera.m_at, m_camera.m_up);
    m_camera.m_cameraUBO.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width) / static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height), 0.1f, 1024.0f);
    m_camera.m_cameraUBO.pos = m_camera.m_pos;
    m_cameraStagingBuffer->Copy(&m_camera.m_cameraUBO);

    vkn::Command::Begin(m_cameraCommandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::CopyBufferToBuffer(m_cameraCommandBuffers[vkn::Sync::GetCurrentFrameIndex()], m_cameraStagingBuffer->Get().buffer, m_camera.m_cameraBuffer->Get().buffer, m_cameraStagingBuffer->Get().bufferInfo.size);
    m_cameraCommandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_cameraCommandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
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
            m_shadowCubemaps[i]->DrawShadowMap(i, m_pointLights, m_meshes);
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
            mesh.m_meshInstanceBuffer.reset();
            mesh.m_meshInstanceBuffer = std::make_unique<vkn::Buffer>(bufferInput);
            mesh.m_meshInstanceBuffer->Copy(mesh.m_meshInstanceUBOs.data());

            bufferInfos.emplace_back(mesh.m_meshInstanceBuffer->Get().descriptorBufferInfo);
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
    meshRenderPipeline.m_prefilteredCubemapDescriptor = m_prefilteredCubemap->m_mipmapDescriptorImageInfo;
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
    m_physics.Stop(m_meshes);
    Unselect();

    m_sceneFilePath.clear();
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
    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_envMap->InsertDummyImage(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()], { 128, 128, 128, 255 });
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Command::SubmitAndWait(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);

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
    m_selectedMeshID = -1;
    m_selectedMeshInstanceID = -1;

    m_physics.Simulate(m_meshes);
    m_meshDirtyFlag = true;
}

void Scene::Stop()
{
    m_physics.Stop(m_meshes);
    m_meshDirtyFlag = true;
}

void Scene::Unselect()
{
    m_selectedMeshID = -1;
    m_selectedMeshInstanceID = -1;
    m_selectedLightID = -1;
}

Scene::~Scene()
{
    vkn::Device::Get().device.destroyCommandPool(ShadowCubemap::s_commandPool);
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    for (auto& imageLoadCommandPool : m_imageLoadCommandPools)
        vkn::Device::Get().device.destroyCommandPool(imageLoadCommandPool);
}
