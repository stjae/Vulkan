#include "scene.h"
#include "../engine/script/script.h"

void Scene::Init()
{
    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);
    vkn::Command::Begin(m_commandBuffer);
    vkn::Image::CreateSampler();
    Script::Init(this);
    CreateCommandBuffers();
    CreateMainCamera();
    CreateShadowMap();
    ShadowCubemap::CreateProjBuffer();
    CreateEnvironmentMap();
    CreateDummyEnvMap();
    SelectDummyEnvMap();
    Physics::InitPhysics();
    vkn::Command::End(m_commandBuffer);
    vkn::Command::SubmitAndWait(m_commandBuffer);
}

void Scene::CreateCommandBuffers()
{
    for (int i = 0; i < 4; i++) {
        vkn::Command::CreateCommandPool(m_imageLoadCommandPools[i]);
        vkn::Command::AllocateCommandBuffer(m_imageLoadCommandPools[i], m_imageLoadCommandBuffers[i]);
    }
}

void Scene::CreateMainCamera()
{
    m_mainCamera.m_dir = { 0.5, -0.3, -0.7 };
    m_mainCamera.m_pos = { -3.0, 3.3, 8.0 };
    m_mainCamera.m_at = { -2.5, 3.0, 7.3 };
    UpdateCameraDescriptor(&m_mainCamera);
}

void Scene::CreateShadowMap()
{
    m_pointLight.Create();
    m_cascadedShadowMap.Create();
    vk::DescriptorImageInfo samplerInfo(vkn::Image::s_repeatSampler);
    meshRenderPipeline.UpdateSampler(samplerInfo);
}

void Scene::CreateEnvironmentMap()
{
    m_envCube.CreateCube();
    m_envCube.CreateBuffers(m_commandBuffer);
    m_envCube.m_meshInstances.push_back(std::make_unique<MeshInstance>(0, MeshInstanceUBO(0, 0)));
    m_brdfLutSquare.CreateSquare();
    m_brdfLutSquare.CreateBuffers(m_commandBuffer);
    m_brdfLutSquare.m_meshInstances.push_back(std::make_unique<MeshInstance>(0, MeshInstanceUBO(0, 0)));
    m_brdfLut.CreateImage({ 512, 512, 1 }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);
    m_brdfLut.CreateImageView();
    m_brdfLut.CreateFramebuffer(brdfLutPipeline);
    m_brdfLut.ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_brdfLut.Draw(m_commandBuffer, m_brdfLutSquare, brdfLutPipeline);
    m_brdfLut.ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    meshRenderPipeline.UpdateBrdfLut(m_brdfLut.Get().descriptorImageInfo);
}

void Scene::UpdateCameraDescriptor(Camera* camera)
{
    assert(camera != nullptr);
    meshRenderPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    colorIDPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    skyboxRenderPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    lineRenderPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
}

void Scene::AddResource(std::string& filePath)
{
    m_resources.emplace_back(filePath);
    m_meshes.push_back(std::make_shared<Mesh>(m_meshes.size(), m_resources.back().filePath));
    m_meshes.back()->CreateBuffers(m_commandBuffer);
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
    meshInstance.physicsDebugDrawer = std::make_unique<PhysicsDebugDrawer>(m_commandBuffer, physicsInfo, mesh.m_indexContainers, mesh.m_vertexContainers);
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
    meshInstance.physicsDebugUBO = {};
}

void Scene::Update()
{
    HandlePointLightDuplication();
    HandleMeshDuplication();

    UpdateViewportCamera();
    UpdatePlayCamera();
    UpdatePointLight();
    UpdatePhysicsDebug();
    UpdateShadowMap();
}

void Scene::AddPointLight()
{
    m_pointLight.Add();
    m_shadowCubemaps.emplace_back();
    m_shadowCubemaps.back() = std::make_unique<ShadowCubemap>();
    m_shadowCubemaps.back()->CreateShadowMap(m_commandBuffer);
    UpdatePointLightBuffer();
}

void Scene::AddEnvironmentMap(const std::string& hdriFilePath)
{
    m_hdriFilePath = hdriFilePath;
    m_envMap = std::make_unique<vkn::Image>();
    m_envMap->InsertHDRImage(hdriFilePath, vk::Format::eR32G32B32A32Sfloat, m_commandBuffer);
    envCubemapPipeline.UpdateHDRimage(m_envMap->Get().descriptorImageInfo);
    UpdateEnvCubemaps();
}

void Scene::SelectDummyEnvMap()
{
    envCubemapPipeline.UpdateHDRimage(m_dummyEnvMap->Get().descriptorImageInfo);
    UpdateEnvCubemaps();
}

void Scene::CreateDummyEnvMap()
{
    m_dummyEnvMap = std::make_unique<vkn::Image>();
    m_dummyEnvMap->InsertDummyImage(m_commandBuffer, { 128, 128, 128, 255 });
}

void Scene::UpdateEnvCubemaps()
{
    m_envCubemap = std::make_unique<EnvCubemap>();
    m_envCubemap->CreateEnvCubemap(m_commandBuffer, 1024, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, envCubemapPipeline);
    m_envCubemap->ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_envCubemap->CreateMipmap(m_commandBuffer);
    m_envCubemap->m_mipmap.ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    m_envCubemap->DrawMipmap(m_commandBuffer, m_envCube, envCubemapPipeline);
    irradianceCubemapPipeline.UpdateEnvCubemap(m_envCubemap->m_mipmapDescriptorImageInfo);
    m_irradianceCubemap = std::make_unique<EnvCubemap>();
    m_irradianceCubemap->CreateEnvCubemap(m_commandBuffer, 32, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, irradianceCubemapPipeline);
    m_irradianceCubemap->ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_irradianceCubemap->DrawEnvCubemap(m_commandBuffer, 32, m_envCube, irradianceCubemapPipeline);
    m_irradianceCubemap->ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    prefilteredCubemapPipeline.UpdateEnvCubemap(m_envCubemap->m_mipmapDescriptorImageInfo);
    m_prefilteredCubemap = std::make_unique<PrefilteredCubemap>();
    m_prefilteredCubemap->CreateEnvCubemap(m_commandBuffer, 256, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, prefilteredCubemapPipeline);
    m_prefilteredCubemap->ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_prefilteredCubemap->CreateMipmap(m_commandBuffer);
    m_prefilteredCubemap->m_mipmap.ChangeImageLayout(m_commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    m_prefilteredCubemap->DrawMipmap(m_commandBuffer, m_envCube, prefilteredCubemapPipeline);
    skyboxRenderPipeline.UpdateIrradianceCubemap(m_irradianceCubemap->Get().descriptorImageInfo);
    meshRenderPipeline.UpdateIrraianceCubemap(m_irradianceCubemap->Get().descriptorImageInfo);
    meshRenderPipeline.UpdatePrefilteredCubemap(m_prefilteredCubemap->m_mipmapDescriptorImageInfo);
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
    m_pointLight.Delete(m_selectedLightIndex);
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
    if (m_selectedLightIndex > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_mainCamera.m_isControllable) {
        m_pointLight.Duplicate(m_selectedLightIndex);
        m_selectedLightIndex = (int)m_pointLight.Size() - 1;
        m_shadowCubemaps.emplace_back();
        m_shadowCubemaps.back() = std::make_unique<ShadowCubemap>();
        m_shadowCubemaps.back()->CreateShadowMap(m_commandBuffer);
        UpdatePointLightBuffer();
    }
}

void Scene::HandleMeshDuplication()
{
    if (m_selectedMeshID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_mainCamera.m_isControllable) {
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

void Scene::UpdateViewportCamera()
{
    if (m_viewportCamera == nullptr)
        m_viewportCamera = &m_mainCamera;
    if (IsPlaying())
        return;
    m_viewportCamera->Control();
    m_viewportCamera->Update(m_commandBuffer);
}

void Scene::UpdatePlayCamera()
{
    if (m_playCamera == nullptr)
        m_playCamera = &m_mainCamera;
    if (!IsPlaying())
        return;
    m_playCamera->Control();
    m_playCamera->Update(m_commandBuffer);
}

void Scene::UpdatePointLight()
{
    m_pointLight.Update(m_commandBuffer);
    for (int i = 0; i < m_pointLight.Size(); i++) {
        m_shadowCubemaps[i]->DrawShadowMap(i, m_pointLight, m_meshes, m_commandBuffer);
    }
}

void Scene::UpdatePointLightBuffer()
{
    m_pointLight.UpdateBuffer();
    UpdateShadowCubemaps();
}

void Scene::UpdateShadowMap()
{
    Camera* camera;
    if (!IsPlaying())
        camera = m_viewportCamera;
    else
        camera = m_playCamera;
    m_cascadedShadowMap.UpdateCascades(camera, m_dirLight);
    m_cascadedShadowMap.UpdateUBO(m_dirLight, m_commandBuffer);
    m_cascadedShadowMap.Draw(m_meshes, m_commandBuffer);
}

void Scene::UpdateShadowCubemaps()
{
    std::vector<vk::DescriptorImageInfo> imageInfos;
    for (auto& shadowCubemap : m_shadowCubemaps) {
        imageInfos.reserve(m_shadowCubemaps.size());
        imageInfos.emplace_back(nullptr, shadowCubemap->Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
    }
    if (!imageInfos.empty())
        meshRenderPipeline.UpdateShadowCubemap(imageInfos);
}

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
        meshRenderPipeline.UpdateMeshUBO(bufferInfos);
        colorIDPipeline.UpdateMeshUBO(bufferInfos);
        shadowMapPipeline.UpdateMeshUBOBuffer(bufferInfos);
        shadowCubemapPipeline.UpdateMeshUBO(bufferInfos);
    }
}

void Scene::UpdatePhysicsDebug()
{
    if (m_selectedMeshID > -1 && m_selectedMeshInstanceID > -1) {
        GetSelectedMeshInstance().physicsDebugUBOBuffer->Copy(&GetSelectedMeshInstance().physicsDebugUBO);
        lineRenderPipeline.UpdateMeshUBO(GetSelectedMeshInstance().physicsDebugUBOBuffer->Get().descriptorBufferInfo);
    }
}

void Scene::UpdateTextureDescriptors()
{
    std::vector<vk::DescriptorImageInfo> imageInfos;
    imageInfos.reserve(m_albedoTextures.size());
    for (const auto& albedoTexture : m_albedoTextures) {
        imageInfos.push_back(albedoTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateAlbedoTextures(imageInfos);
    imageInfos.clear();
    imageInfos.reserve(m_normalTextures.size());
    for (const auto& normalTexture : m_normalTextures) {
        imageInfos.push_back(normalTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateNormalTextures(imageInfos);
    imageInfos.clear();
    imageInfos.reserve(m_metallicTextures.size());
    for (const auto& metallicTexture : m_metallicTextures) {
        imageInfos.push_back(metallicTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateMetallicTextures(imageInfos);
    imageInfos.clear();
    imageInfos.reserve(m_roughnessTextures.size());
    for (const auto& roughnessTexture : m_roughnessTextures) {
        imageInfos.push_back(roughnessTexture->Get().descriptorImageInfo);
    }
    meshRenderPipeline.UpdateRoughnessTextures(imageInfos);
}

void Scene::Clear()
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
    m_pointLight.Clear();
    m_shadowCubemaps.clear();
    m_albedoTextures.clear();
    m_normalTextures.clear();
    m_metallicTextures.clear();
    m_roughnessTextures.clear();
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
            if (instance->camera.lock())
                m_meshCopies.back().m_meshInstances.back()->camera = instance->camera;
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
            if (m_meshCopies[i].m_meshInstances[j]->physicsInfo) {
                AddPhysics(*m_meshes[i], *m_meshes[i]->m_meshInstances.back(), *m_meshCopies[i].m_meshInstances[j]->physicsInfo);
                m_meshes[i]->m_meshInstances.back()->physicsDebugUBO.scale = m_meshCopies[i].m_meshInstances[j]->physicsInfo->scale;
            }
            if (m_meshCopies[i].m_meshInstances[j]->camera.lock()) {
                m_meshes[i]->m_meshInstances.back()->camera = m_meshCopies[i].m_meshInstances[j]->camera;
            }
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
        for (auto& scriptInstance : Script::s_scriptInstances) {
            scriptInstance.second->InvokeOnCreate();
        }
        m_playCamera->Init();
        UpdateCameraDescriptor(m_playCamera);
    }

    Physics::Simulate(m_meshes);

    for (auto& scriptInstance : Script::s_scriptInstances) {
        scriptInstance.second->InvokeOnUpdate(Time::GetDeltaTime());
    }
}

void Scene::Stop()
{
    UpdateCameraDescriptor(m_viewportCamera);

    for (auto& mesh : m_meshes) {
        for (auto& instance : mesh->m_meshInstances) {
            if (instance->physicsInfo)
                DeletePhysics(*instance);
        }
    }
    for (auto& scriptInstance : Script::s_scriptInstances) {
        scriptInstance.second->InvokeOnDestroy();
    }
    m_isStartUp = true;
    RevertMeshInstances();
    UpdateMeshBuffer();
}

void Scene::UnselectAll()
{
    m_selectedMeshID = -1;
    m_selectedMeshInstanceID = -1;
    m_selectedLightIndex = -1;
}

Scene::~Scene()
{
    ShadowCubemap::DestroyBuffer();
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    for (auto& imageLoadCommandPool : m_imageLoadCommandPools)
        vkn::Device::Get().device.destroyCommandPool(imageLoadCommandPool);
}

void Scene::AddCamera(MeshInstance& instance)
{
    m_subCameras.emplace(instance.UUID, std::make_shared<SubCamera>(instance.UUID));
    instance.camera = m_subCameras[instance.UUID];
}

_Resource::_Resource(std::string& path)
{
    this->filePath = path;
    this->fileName = path.substr(path.find_last_of("/\\") + 1, path.rfind('.') - path.find_last_of("/\\") - 1);
}
