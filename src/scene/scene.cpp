#include "scene.h"
#include "../engine/script/script.h"

Scene::Scene() : m_envCube(0), m_brdfLutSquare(0)
{
    Script::Init(this);

    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffers);
    m_mainCamera = std::make_unique<MainCamera>(m_commandPool);

    for (int i = 0; i < 4; i++) {
        vkn::Command::CreateCommandPool(m_imageLoadCommandPools[i]);
        vkn::Command::AllocateCommandBuffer(m_imageLoadCommandPools[i], m_imageLoadCommandBuffers[i]);
    }

    {
        // init camera pos
        m_mainCamera->m_dir = { 0.5, -0.3, -0.7 };
        m_mainCamera->m_pos = { -3.0, 3.3, 8.0 };
        m_mainCamera->m_at = { -2.5, 3.0, 7.3 };

        UpdateCameraDescriptorSet(m_mainCamera.get());
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
        shadowMapPipeline.UpdateShadowMapSpaceViewProjDescriptor();
        meshRenderPipeline.m_shadowMapSpaceViewProjDescriptor = m_shadowMapViewSpaceProjBuffer->Get().descriptorBufferInfo;
        meshRenderPipeline.UpdateShadowMapSpaceViewProjDescriptor();
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
        m_envCube.m_meshInstances.push_back(std::make_unique<MeshInstance>(0, MeshInstanceUBO(0, 0)));
        m_brdfLutSquare.CreateSquare();
        m_brdfLutSquare.CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        m_brdfLutSquare.m_meshInstances.push_back(std::make_unique<MeshInstance>(0, MeshInstanceUBO(0, 0)));
        m_brdfLut.CreateImage({ 512, 512, 1 }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);
        m_brdfLut.CreateImageView();
        m_brdfLut.CreateFramebuffer(brdfLutPipeline);
        m_brdfLut.Draw(m_brdfLutSquare, brdfLutPipeline, m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
        meshRenderPipeline.m_brdfLutDescriptor = m_brdfLut.Get().descriptorImageInfo;
        meshRenderPipeline.UpdateBrdfLutDescriptor();

        Physics::InitPhysics();
    }

    InitHdri();
}

void Scene::UpdateCameraDescriptorSet(Camera* camera)
{
    assert(camera != nullptr);
    meshRenderPipeline.m_cameraDescriptor = camera->m_cameraBuffer->Get().descriptorBufferInfo;
    meshRenderPipeline.UpdateCameraDescriptor();
    skyboxRenderPipeline.m_cameraDescriptor = camera->m_cameraBuffer->Get().descriptorBufferInfo;
    skyboxRenderPipeline.UpdateCameraDescriptor();
    lineRenderPipeline.m_cameraDescriptor = camera->m_cameraBuffer->Get().descriptorBufferInfo;
    lineRenderPipeline.UpdateCameraDescriptor();
}

void Scene::AddResource(std::string& filePath)
{
    m_resources.emplace_back(filePath);
    m_meshes.push_back(std::make_shared<Mesh>(m_meshes.size(), m_resources.back().filePath));
    m_meshes.back()->CreateBuffers(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    m_resources.back().ptr = m_meshes.back();

    if (!m_meshes.back()->m_materials.empty()) {
        LoadMaterials(filePath, m_meshes.back()->m_materials);
        UpdateTextureDescriptors();
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

void Scene::AddMeshInstance(Mesh& mesh, glm::vec3 pos, glm::vec3 scale)
{
    mesh.AddInstance(pos, scale);
    m_meshInstanceMap[mesh.m_meshInstances.back()->UUID] = mesh.m_meshInstances.back().get();
    UpdateMeshBuffer();
}

void Scene::AddMeshInstance(Mesh& mesh, const uint64_t UUID)
{
    mesh.AddInstance(UUID);
    m_meshInstanceMap[mesh.m_meshInstances.back()->UUID] = mesh.m_meshInstances.back().get();
    UpdateMeshBuffer();
}

void Scene::AddPhysics(Mesh& mesh, MeshInstance& meshInstance, PhysicsInfo& physicsInfo)
{
    meshInstance.physicsDebugDrawer = std::make_unique<PhysicsDebugDrawer>(physicsInfo, mesh.m_indexContainers, mesh.m_vertexContainers);
    meshInstance.physicsInfo = std::make_unique<PhysicsInfo>(physicsInfo);
    meshInstance.physicsDebugUBO.havePhysicsInfo = 1;
    Physics::AddRigidBody(mesh, meshInstance);
    meshInstance.physicsDebugUBO.model = meshInstance.UBO.model;
}

void Scene::DeletePhysics(MeshInstance& meshInstance)
{
    if (!meshInstance.physicsInfo)
        return;
    Physics::DeleteRigidBody(meshInstance);
    meshInstance.physicsDebugDrawer.reset();
    meshInstance.physicsInfo.reset();
    meshInstance.physicsDebugUBO.havePhysicsInfo = 0;
}

void Scene::Update()
{
    HandlePointLightDuplication();
    HandleMeshDuplication();

    UpdateMainCamera();
    UpdatePointLight();
    UpdatePhysicsDebug();
    UpdateShadowMap();
}

void Scene::AddPointLight()
{
    m_pointLights.emplace_back();
    m_shadowCubemaps.emplace_back();
    m_shadowCubemaps.back() = std::make_unique<ShadowCubemap>();
    m_shadowCubemaps.back()->CreateShadowMap();
    UpdatePointLightBuffer();
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
    UpdateEnvCubemapDescriptors();
}

void Scene::DeleteMeshInstance(Mesh& mesh, MeshInstance& instance)
{
    DeletePhysics(instance);
    mesh.DeleteInstance(instance.UBO.instanceColorID);
    m_meshInstanceMap.erase(instance.UUID);
    UnselectAll();
    UpdateMeshBuffer();
}

void Scene::DeletePointLight()
{
    m_pointLights.erase(m_pointLights.begin() + m_selectedLightID);
    UnselectAll();
    UpdatePointLightBuffer();
}

size_t Scene::GetInstanceCount()
{
    size_t instanceCount = 0;
    for (auto& mesh : m_meshes) {
        instanceCount += mesh->GetInstanceCount();
    }
    return instanceCount;
}

void Scene::SelectByColorID(int32_t meshID, int32_t instanceID)
{
    UnselectAll();
    m_selectedMeshID = meshID;
    m_selectedMeshInstanceID = instanceID;
}

void Scene::HandlePointLightDuplication()
{
    if (m_selectedLightID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_mainCamera->m_isControllable) {
        AddPointLight();
        m_pointLights.back() = m_pointLights[m_selectedLightID];
        m_selectedLightID = m_pointLights.size() - 1;
    }
}

void Scene::HandleMeshDuplication()
{
    if (m_selectedMeshID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_mainCamera->m_isControllable) {
        auto& mesh = GetSelectedMesh();
        auto& srcInstance = GetSelectedMeshInstance();

        AddMeshInstance(mesh);
        auto& newInstance = *mesh.m_meshInstances.back();
        int32_t newColorID = newInstance.UBO.instanceColorID;
        newInstance = srcInstance;
        // Copy except for color id
        newInstance.UBO.instanceColorID = newColorID;
        // Copy physics information
        if (srcInstance.physicsInfo) {
            AddPhysics(mesh, newInstance, *srcInstance.physicsInfo);
        }
        // Select new instance
        m_selectedMeshInstanceID = newInstance.UBO.instanceColorID;
        // Update
        mesh.UpdateUBO(GetSelectedMeshInstance());
    }
}

void Scene::UpdateMainCamera()
{
    if (m_isPlaying && m_selectedCameraUUID != 0)
        return;
    m_mainCamera->Control();
    m_mainCamera->Update();
}

void Scene::UpdatePointLight()
{
    if (m_pointLights.empty())
        return;

    m_pointLightDataBuffer->Copy(m_pointLights.data());

    for (int i = 0; i < m_pointLights.size(); i++) {
        m_shadowCubemaps[i]->DrawShadowMap(i, m_pointLights, m_meshes);
    }
}

void Scene::UpdatePointLightBuffer()
{
    // TODO: separate
    if (!m_pointLights.empty()) {
        vkn::BufferInfo bufferInput = { sizeof(PointLightUBO) * m_pointLights.size(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        m_pointLightDataBuffer.reset();
        m_pointLightDataBuffer = std::make_unique<vkn::Buffer>(bufferInput);
        m_pointLightDataBuffer->Copy(m_pointLights.data());
        meshRenderPipeline.m_pointLightDescriptor = m_pointLightDataBuffer->Get().descriptorBufferInfo;
        shadowCubemapPipeline.m_pointLightDescriptor = m_pointLightDataBuffer->Get().descriptorBufferInfo;
        meshRenderPipeline.UpdatePointLightDescriptor();
        shadowCubemapPipeline.UpdatePointLightDescriptor();

        UpdateShadowCubemaps();
    }
}

void Scene::UpdateShadowMap()
{
    // TODO: follow camera
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
    for (int i = 0; i < m_pointLights.size(); i++) {
        m_shadowCubemaps[i]->DrawShadowMap(i, m_pointLights, m_meshes);
    }
    meshRenderPipeline.m_shadowCubemapDescriptors.clear();
    for (auto& shadowCubemap : m_shadowCubemaps) {
        meshRenderPipeline.m_shadowCubemapDescriptors.emplace_back(nullptr, shadowCubemap->Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
    meshRenderPipeline.UpdateShadowCubemapDescriptors();
}

// void Scene::UpdateMeshInstance(Mesh& mesh, MeshInstance& instance)
// {
//     mesh.m_meshInstanceUBOs[instance.UBO.instanceColorID] = instance.UBO;
//     mesh.m_meshInstanceUBOBuffer->Copy(mesh.m_meshInstanceUBOs.data());
//     instance.physicsDebugUBO.model = instance.UBO.model;
// }

void Scene::UpdateMeshBuffer()
{
    if (GetInstanceCount() > 0) {
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(m_meshes.size());
        for (auto& mesh : m_meshes) {
            if (mesh->GetInstanceCount() < 1)
                continue;
            vkn::BufferInfo bufferInfo = { sizeof(MeshInstanceUBO) * mesh->GetInstanceCount(), vk::WholeSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
            mesh->m_meshInstanceUBOBuffer.reset();
            mesh->m_meshInstanceUBOBuffer = std::make_unique<vkn::Buffer>(bufferInfo);
            mesh->m_meshInstanceUBOs.clear();
            for (auto& instance : mesh->m_meshInstances) {
                mesh->m_meshInstanceUBOs.push_back(instance->UBO);
            }
            mesh->m_meshInstanceUBOBuffer->Copy(mesh->m_meshInstanceUBOs.data());
            bufferInfos.emplace_back(mesh->m_meshInstanceUBOBuffer->Get().descriptorBufferInfo);
        }
        meshRenderPipeline.m_meshDescriptors = bufferInfos;
        shadowMapPipeline.m_meshDescriptors = bufferInfos;
        shadowCubemapPipeline.m_meshDescriptors = bufferInfos;
        meshRenderPipeline.UpdateMeshDescriptors();
        shadowMapPipeline.UpdateMeshDescriptors();
        shadowCubemapPipeline.UpdateMeshDescriptors();
    }
}

void Scene::UpdatePhysicsDebug()
{
    if (m_selectedMeshID > -1 && m_selectedMeshInstanceID > -1) {
        GetSelectedMeshInstance().physicsDebugUBOBuffer->Copy(&GetSelectedMeshInstance().physicsDebugUBO);
        lineRenderPipeline.m_UBODescriptor = GetSelectedMeshInstance().physicsDebugUBOBuffer->Get().descriptorBufferInfo;
        lineRenderPipeline.UpdateUBODescriptor();
    }
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
}

void Scene::UpdateEnvCubemapDescriptors()
{
    skyboxRenderPipeline.m_irradianceCubemapDescriptor = m_irradianceCubemap->Get().descriptorImageInfo;
    skyboxRenderPipeline.UpdateIrradianceCubemapDescriptor();
    meshRenderPipeline.m_irradianceCubemapDescriptor = m_irradianceCubemap->Get().descriptorImageInfo;
    meshRenderPipeline.UpdateIrraianceCubemapDescriptor();
    meshRenderPipeline.m_prefilteredCubemapDescriptor = m_prefilteredCubemap->m_mipmapDescriptorImageInfo;
    meshRenderPipeline.UpdatePrefilteredCubemapDescriptor();
}

void Scene::InitScene()
{
    UnselectAll();
    Stop();

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
    UpdateEnvCubemapDescriptors();
}

void Scene::CopyMeshInstances()
{
    m_meshCopies.reserve(m_meshes.size());
    for (auto& mesh : m_meshes) {
        m_meshCopies.emplace_back(mesh->m_meshColorID);
        m_meshCopies.back().m_meshInstances.reserve(mesh->m_meshInstances.size());
        for (auto& instance : mesh->m_meshInstances) {
            AddMeshInstance(m_meshCopies.back(), instance->UUID);
            *m_meshCopies.back().m_meshInstances.back() = *instance;
            m_meshInstanceMap[instance->UUID] = instance.get();
            if (instance->physicsInfo)
                m_meshCopies.back().m_meshInstances.back()->physicsInfo = std::make_unique<PhysicsInfo>(*instance->physicsInfo);
            if (instance->camera)
                m_meshCopies.back().m_meshInstances.back()->camera = std::make_unique<SubCamera>(m_commandPool);
        }
    }
}

void Scene::RevertMeshInstances()
{
    if (m_meshCopies.empty())
        return;

    for (int i = 0; i < m_meshes.size(); i++) {
        m_meshes[i]->m_meshInstances.clear();
        for (int j = 0; j < m_meshCopies[i].m_meshInstances.size(); j++) {
            AddMeshInstance(*m_meshes[i], m_meshCopies[i].m_meshInstances[j]->UUID);
            *m_meshes[i]->m_meshInstances.back() = *m_meshCopies[i].m_meshInstances[j];
            m_meshInstanceMap[m_meshCopies[i].m_meshInstances[j]->UUID] = m_meshCopies[i].m_meshInstances[j].get();
            if (m_meshCopies[i].m_meshInstances[j]->physicsInfo)
                AddPhysics(*m_meshes[i], *m_meshes[i]->m_meshInstances.back(), *m_meshCopies[i].m_meshInstances[j]->physicsInfo);
            if (m_meshCopies[i].m_meshInstances[j]->camera)
                m_meshes[i]->m_meshInstances.back()->camera = std::make_unique<SubCamera>(m_commandPool);
        }
    }
    m_meshCopies.clear();
}

void Scene::Play()
{
    if (!m_isPlaying) {
        return;
    }

    if (m_isStartUp) {
        UnselectAll();
        m_isStartUp = false;
        Physics::UpdateRigidBodies(m_meshes);
        CopyMeshInstances();
        // TODO: select camera here
        // if (m_selectedCameraUUID == 0) {
        //     SelectCamera(m_mainCamera.get());
        // } else {
        //     m_selectedCameraMeshInstance = m_meshInstanceMap[m_selectedCameraUUID];
        //     SelectCamera(m_selectedCameraMeshInstance->camera.get());
        // }
        for (auto& scriptInstance : Script::s_scriptInstances) {
            scriptInstance.second->InvokeOnCreate();
        }
    }

    if (m_selectedCameraUUID != 0) {
        m_selectedCameraMeshInstance->camera->ControlByMatrix(m_selectedCameraMeshInstance->UBO.model);
        m_selectedCameraMeshInstance->camera->Update();
    }

    Physics::Simulate(m_meshes);

    for (auto& scriptInstance : Script::s_scriptInstances) {
        scriptInstance.second->InvokeOnUpdate(Time::GetDeltaTime());
    }
}

void Scene::Stop()
{
    SelectCamera(m_mainCamera.get());

    for (auto& mesh : m_meshes) {
        for (auto& instance : mesh->m_meshInstances) {
            if (instance->physicsInfo)
                DeletePhysics(*instance);
        }
    }
    m_isStartUp = true;
    RevertMeshInstances();
    UpdateMeshBuffer();
}

void Scene::UnselectAll()
{
    m_selectedMeshID = -1;
    m_selectedMeshInstanceID = -1;
    m_selectedLightID = -1;
}

Scene::~Scene()
{
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    for (auto& imageLoadCommandPool : m_imageLoadCommandPools)
        vkn::Device::Get().device.destroyCommandPool(imageLoadCommandPool);
}

void Scene::AddCamera(MeshInstance& instance)
{
    instance.camera = std::make_unique<SubCamera>(m_commandPool);
}

void Scene::SelectCamera(Camera* camera)
{
    UpdateCameraDescriptorSet(camera);
}