#include "scene.h"
#include "../pipeline/postProcessPipeline.h"
#include "../pipeline/colorIDPipeline.h"
#include "../pipeline/envCubemapPipeline.h"
#include "../pipeline/irradianceCubemapPipeline.h"
#include "../pipeline/prefilteredCubemapPipeline.h"
#include "../pipeline/brdfLutPipeline.h"
#include "../pipeline/skyboxRenderPipeline.h"
#include "../pipeline/lineRenderPipeline.h"
#include "../pipeline/physicsDebugPipeline.h"
#include "../engine/script/script.h"
#include <filesystem>

void Scene::Init(const vk::CommandBuffer& commandBuffer)
{
    vkn::Image::CreateSampler();
    Script::Init(this);
    CreateImageLoadCommandBuffers();
    CreateMainCamera();
    CreateShadowMap();
    ShadowCubemap::CreateProjBuffer();
    CreateEnvironmentMap(commandBuffer);
    CreateDummyEnvMap(commandBuffer);
    SelectDummyEnvMap(commandBuffer);
    Physics::InitPhysics();
    CreateGrid(commandBuffer);
}

void Scene::CreateGrid(const vk::CommandBuffer& commandBuffer)
{
    m_gridWidth = std::max(2, m_gridWidth);
    m_grid.CreateGrid(m_gridWidth);
    m_grid.CreateBuffer();
    m_grid.CopyBuffer(commandBuffer);
}

void Scene::CreateImageLoadCommandBuffers()
{
    for (int i = 0; i < 4; i++) {
        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = vkn::Device::Get().graphicsFamilyIndex.value();
        m_imageLoadCommandPools[i] = vkn::Device::Get().device.createCommandPool(poolInfo);

        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.commandPool = m_imageLoadCommandPools[i];
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = 1;
        m_imageLoadCommandBuffers[i] = vkn::Device::Get().device.allocateCommandBuffers(allocateInfo)[0];
    }
}

void Scene::CreateMainCamera()
{
    m_mainCamera.SetInitPos();
    UpdateCameraDescriptor(&m_mainCamera);
}

void Scene::CreateShadowMap()
{
    m_pointLight.Create();
    m_cascadedShadowMap.Create();
    vk::DescriptorImageInfo samplerInfo(vkn::Image::s_repeatSampler);
    meshRenderPipeline.UpdateSampler(samplerInfo);
}

void Scene::CreateEnvironmentMap(const vk::CommandBuffer& commandBuffer)
{
    m_cube.CreateCube();
    m_cube.CreateBuffers(commandBuffer);
    m_cube.m_meshInstances.push_back(std::make_unique<MeshInstance>(0, MeshInstanceUBO(0, 0)));
    m_square.CreateSquare();
    m_square.CreateBuffers(commandBuffer);
    m_square.m_meshInstances.push_back(std::make_unique<MeshInstance>(0, MeshInstanceUBO(0, 0)));
    m_brdfLut.CreateImage({ 512, 512, 1 }, vk::Format::eR16G16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_clampSampler);
    m_brdfLut.CreateImageView();
    m_brdfLut.CreateFramebuffer(brdfLutPipeline);
    m_brdfLut.ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_brdfLut.Draw(commandBuffer, m_square, brdfLutPipeline);
    m_brdfLut.ChangeImageLayout(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    meshRenderPipeline.UpdateBrdfLut(m_brdfLut.Get().descriptorImageInfo);
}

void Scene::UpdateCameraDescriptor(Camera* camera)
{
    assert(camera != nullptr);
    meshRenderPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    postProcessPipeline.UpdatePrevCameraUBO(camera->m_prevCameraBuffer->Get().descriptorBufferInfo);
    postProcessPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    colorIDPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    skyboxRenderPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    lineRenderPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
    physicsDebugPipeline.UpdateCameraUBO(camera->m_cameraBuffer->Get().descriptorBufferInfo);
}

void Scene::AddResource(const vk::CommandBuffer& commandBuffer, std::string& filePath)
{
    std::string resourceName = filePath.substr(filePath.find_last_of("/\\") + 1, filePath.rfind('.') - filePath.find_last_of("/\\") - 1);
    std::string resourcePath = "\\resource\\" + resourceName + filePath.substr(filePath.find_last_of("/\\"), filePath.length() - 1);
    m_resources.emplace_back(resourcePath, resourceName);

    std::error_code ec;
    std::filesystem::create_directories(m_sceneFolderPath + "\\resource\\" + resourceName, ec);
    Log(DEBUG, fmt::terminal_color::bright_black, "create resource directory: {0}, {1}", m_sceneFolderPath + "\\resource\\" + resourceName, ec.message());
    std::filesystem::copy(filePath, m_sceneFolderPath + "\\resource\\" + resourceName, ec);
    Log(DEBUG, fmt::terminal_color::bright_black, "copy resource: {0}, {1}", filePath, ec.message());
    std::string binPath = filePath.substr(0, filePath.rfind('.')) + ".bin";
    std::filesystem::copy(binPath, m_sceneFolderPath + "\\resource\\" + resourceName, ec);
    Log(DEBUG, fmt::terminal_color::bright_black, "copy resource: {0}, {1}", binPath, ec.message());

    m_meshes.push_back(std::make_unique<Mesh>(m_meshes.size(), filePath, resourceName));
    m_meshes.back()->CreateBuffers(commandBuffer);
    m_resources.back().ptr = m_meshes.back().get();

    if (!m_meshes.back()->m_materials.empty()) {
        LoadMaterials(m_meshes.back()->m_filePath, m_meshes.back()->m_name, m_meshes.back()->m_materials);
        UpdateTextureDescriptors();
    }
}

void Scene::LoadMaterials(const std::string& modelPath, const std::string& modelName, const std::vector<MaterialFilePath>& materials)
{
    for (int i = 0; i < 4; i++) {
        vk::CommandBufferUsageFlags flag = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        vk::CommandBufferBeginInfo beginInfo(flag, {});
        vkn::CHECK_RESULT(m_imageLoadCommandBuffers[i].begin(&beginInfo));
    }

    std::thread t0 = std::thread([&]() {
        for (auto& material : materials) {
            m_albedoTextures.emplace_back();
            m_albedoTextures.back() = std::make_unique<vkn::Image>();
            if (m_albedoTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.albedo, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[0])) {
                std::error_code ec;
                auto texturePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.albedo;
                auto textureFolderPath = m_sceneFolderPath + "\\resource\\" + modelName + "\\" + material.albedo.substr(0, material.albedo.find_last_of("/\\") + 1);
                std::filesystem::create_directories(textureFolderPath);
                std::filesystem::copy(texturePath, textureFolderPath, ec);
                Log(DEBUG, fmt::terminal_color::bright_black, "copy albedo texture: {0}, {1}", texturePath, ec.message());
            }
        }
    });
    std::thread t1 = std::thread([&]() {
        for (auto& material : materials) {
            m_normalTextures.emplace_back();
            m_normalTextures.back() = std::make_unique<vkn::Image>();
            if (m_normalTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normal, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[1])) {
                std::error_code ec;
                auto texturePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.normal;
                auto textureFolderPath = m_sceneFolderPath + "\\resource\\" + modelName + "\\" + material.normal.substr(0, material.normal.find_last_of("/\\") + 1);
                std::filesystem::create_directories(textureFolderPath);
                std::filesystem::copy(texturePath, textureFolderPath, ec);
                Log(DEBUG, fmt::terminal_color::bright_black, "copy normal texture: {0}, {1}", texturePath, ec.message());
            }
        }
    });
    std::thread t2 = std::thread([&]() {
        for (auto& material : materials) {
            m_metallicTextures.emplace_back();
            m_metallicTextures.back() = std::make_unique<vkn::Image>();
            if (m_metallicTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.metallic, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[2])) {
                std::error_code ec;
                auto texturePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.metallic;
                auto textureFolderPath = m_sceneFolderPath + "\\resource\\" + modelName + "\\" + material.metallic.substr(0, material.metallic.find_last_of("/\\") + 1);
                std::filesystem::create_directories(textureFolderPath);
                std::filesystem::copy(texturePath, textureFolderPath, ec);
                Log(DEBUG, fmt::terminal_color::bright_black, "copy metallic texture: {0}, {1}", texturePath, ec.message());
            }
        }
    });
    std::thread t3 = std::thread([&]() {
        for (auto& material : materials) {
            m_roughnessTextures.emplace_back();
            m_roughnessTextures.back() = std::make_unique<vkn::Image>();
            if (m_roughnessTextures.back()->InsertImage(modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.roughness, vk::Format::eR8G8B8A8Unorm, m_imageLoadCommandBuffers[3])) {
                std::error_code ec;
                auto texturePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1) + material.roughness;
                auto textureFolderPath = m_sceneFolderPath + "\\resource\\" + modelName + "\\" + material.roughness.substr(0, material.roughness.find_last_of("/\\") + 1);
                std::filesystem::create_directories(textureFolderPath);
                std::filesystem::copy(texturePath, textureFolderPath, ec);
                Log(DEBUG, fmt::terminal_color::bright_black, "copy roughness texture: {0}, {1}", texturePath, ec.message());
            }
        }
    });

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    for (int i = 0; i < 4; i++)
        m_imageLoadCommandBuffers[i].end();

    for (int i = 0; i < 4; i++) {
        vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &m_imageLoadCommandBuffers[i]);
        vkn::CHECK_RESULT(vkn::Device::Get().device.resetFences(1, &vkn::Sync::GetCommandFence()));
        vkn::CHECK_RESULT(vkn::Device::Get().graphicsQueue.submit(1, &submitInfo, vkn::Sync::GetCommandFence()));
        vkn::CHECK_RESULT(vkn::Device::Get().device.waitForFences(1, &vkn::Sync::GetCommandFence(), VK_TRUE, UINT64_MAX));
    }
}

void Scene::AddMeshInstance(Mesh& mesh, glm::vec3 pos, glm::vec3 scale)
{
    mesh.AddInstance(pos, scale);
    m_meshInstances[mesh.m_meshInstances.back()->UUID] = mesh.m_meshInstances.back().get();
    UpdateMeshBuffer();
}

void Scene::AddMeshInstance(Mesh& mesh, const uint64_t UUID)
{
    mesh.AddInstance(UUID);
    m_meshInstances[mesh.m_meshInstances.back()->UUID] = mesh.m_meshInstances.back().get();
    UpdateMeshBuffer();
}

void Scene::AddPhysics(const vk::CommandBuffer& commandBuffer, Mesh& mesh, MeshInstance& meshInstance, PhysicsInfo& physicsInfo)
{
    meshInstance.physicsDebugDrawer = std::make_unique<PhysicsDebugDrawer>(commandBuffer, physicsInfo, mesh.m_indexContainers, mesh.m_vertexContainers);
    meshInstance.physicsInfo = std::make_unique<PhysicsInfo>(physicsInfo);
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

void Scene::Update(const vk::CommandBuffer& commandBuffer)
{
    if (m_selectedLightIndex > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_mainCamera.m_isControllable)
        DuplicatePointLight(commandBuffer, m_selectedLightIndex);
    if (m_selectedMeshID > -1 && ImGui::IsKeyPressed(ImGuiKey_D, false) && !m_mainCamera.m_isControllable) {
        DuplicateMeshInstance(commandBuffer, m_selectedMeshID, m_selectedMeshInstanceID, { 0.0f, 0.0f, 0.0f });
        m_selectedMeshInstanceID = m_meshes[m_selectedMeshID]->m_meshInstances.back()->UBO.instanceColorID;
    }

    UpdateViewportCamera(commandBuffer);
    UpdatePlayCamera(commandBuffer);
    UpdatePointLight(commandBuffer);
    UpdatePhysicsDebug();
    UpdateShadowMap(commandBuffer);
}

void Scene::AddPointLight(const vk::CommandBuffer& commandBuffer)
{
    m_pointLight.Add();
    m_shadowCubemaps.emplace_back();
    m_shadowCubemaps.back() = std::make_unique<ShadowCubemap>();
    m_shadowCubemaps.back()->CreateShadowMap(commandBuffer);
    UpdatePointLightBuffer();
}

void Scene::AddEnvironmentMap(const vk::CommandBuffer& commandBuffer, const std::string& hdriFilePath)
{
    std::error_code ec;
    std::filesystem::create_directory(m_sceneFolderPath + "\\hdri", ec);
    Log(DEBUG, fmt::terminal_color::bright_black, "create directory for hdri: {0}, {1}", m_sceneFolderPath + "\\hdri", ec.message());
    std::filesystem::copy(hdriFilePath, m_sceneFolderPath + "\\hdri", ec);
    Log(DEBUG, fmt::terminal_color::bright_black, "copy hdri file: {0}, {1}", hdriFilePath, ec.message());

    m_hdriFilePath = "\\hdri" + hdriFilePath.substr(hdriFilePath.find_last_of("/\\"), hdriFilePath.length() - 1);
    m_envMap = std::make_unique<vkn::Image>();
    m_envMap->InsertHDRImage(m_sceneFolderPath + m_hdriFilePath, vk::Format::eR32G32B32A32Sfloat, commandBuffer);
    envCubemapPipeline.UpdateHDRimage(m_envMap->Get().descriptorImageInfo);
    UpdateEnvCubemaps(commandBuffer);
}

void Scene::SelectDummyEnvMap(const vk::CommandBuffer& commandBuffer)
{
    envCubemapPipeline.UpdateHDRimage(m_dummyEnvMap->Get().descriptorImageInfo);
    UpdateEnvCubemaps(commandBuffer);
}

void Scene::CreateDummyEnvMap(const vk::CommandBuffer& commandBuffer)
{
    m_dummyEnvMap = std::make_unique<vkn::Image>();
    m_dummyEnvMap->InsertDummyImage(commandBuffer, { 128, 128, 128, 255 });
}

void Scene::UpdateEnvCubemaps(const vk::CommandBuffer& commandBuffer)
{
    m_envCubemap = std::make_unique<EnvCubemap>();
    m_envCubemap->CreateEnvCubemap(commandBuffer, 1024, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, envCubemapPipeline);
    m_envCubemap->ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_envCubemap->CreateMipmap(commandBuffer);
    m_envCubemap->m_mipmap.ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    m_envCubemap->DrawMipmap(commandBuffer, m_cube, envCubemapPipeline);
    irradianceCubemapPipeline.UpdateEnvCubemap(m_envCubemap->m_mipmapDescriptorImageInfo);
    m_irradianceCubemap = std::make_unique<EnvCubemap>();
    m_irradianceCubemap->CreateEnvCubemap(commandBuffer, 32, vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, irradianceCubemapPipeline);
    m_irradianceCubemap->ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_irradianceCubemap->DrawEnvCubemap(commandBuffer, 32, m_cube, irradianceCubemapPipeline);
    m_irradianceCubemap->ChangeImageLayout(commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    prefilteredCubemapPipeline.UpdateEnvCubemap(m_envCubemap->m_mipmapDescriptorImageInfo);
    m_prefilteredCubemap = std::make_unique<PrefilteredCubemap>();
    m_prefilteredCubemap->CreateEnvCubemap(commandBuffer, 256, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, prefilteredCubemapPipeline);
    m_prefilteredCubemap->ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
    m_prefilteredCubemap->CreateMipmap(commandBuffer);
    m_prefilteredCubemap->m_mipmap.ChangeImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    m_prefilteredCubemap->DrawMipmap(commandBuffer, m_cube, prefilteredCubemapPipeline);
    skyboxRenderPipeline.UpdateIrradianceCubemap(m_irradianceCubemap->Get().descriptorImageInfo);
    meshRenderPipeline.UpdateIrraianceCubemap(m_irradianceCubemap->Get().descriptorImageInfo);
    meshRenderPipeline.UpdatePrefilteredCubemap(m_prefilteredCubemap->m_mipmapDescriptorImageInfo);
}

void Scene::DeleteMeshInstance(Mesh& mesh, MeshInstance& instance)
{
    DeletePhysics(instance);
    DeleteCamera(instance.UUID);
    Script::s_scriptInstances.erase(instance.UUID);
    mesh.DeleteInstance(instance.UBO.instanceColorID);
    m_meshInstances.erase(instance.UUID);
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

void Scene::DuplicatePointLight(const vk::CommandBuffer& commandBuffer, int lightIndex)
{
    if (lightIndex < 0)
        return;
    m_pointLight.Duplicate(lightIndex);
    m_selectedLightIndex = (int)m_pointLight.Size() - 1;
    m_shadowCubemaps.emplace_back();
    m_shadowCubemaps.back() = std::make_unique<ShadowCubemap>();
    m_shadowCubemaps.back()->CreateShadowMap(commandBuffer);
    UpdatePointLightBuffer();
}

void Scene::DuplicateMeshInstance(const vk::CommandBuffer& commandBuffer, int32_t meshID, int32_t meshInstanceID, glm::vec3 offset)
{
    if (meshID < 0 || meshInstanceID < 0)
        return;
    auto& mesh = *m_meshes[meshID];
    auto& srcInstance = *mesh.m_meshInstances[meshInstanceID];
    AddMeshInstance(mesh);
    auto& newInstance = *mesh.m_meshInstances.back();
    int32_t newColorID = newInstance.UBO.instanceColorID;
    newInstance = srcInstance;
    float m[16];
    ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(newInstance.translation + offset), glm::value_ptr(newInstance.rotation), glm::value_ptr(newInstance.scale), m);
    newInstance.UBO.model = glm::make_mat4(m);
    newInstance.UpdateTransform();
    // Copy except for color id
    newInstance.UBO.instanceColorID = newColorID;
    // Copy physics information
    if (srcInstance.physicsInfo) {
        AddPhysics(commandBuffer, mesh, newInstance, *srcInstance.physicsInfo);
    }
    m_meshes[meshID]->UpdateUBO(newInstance);
}

void Scene::UpdateViewportCamera(const vk::CommandBuffer& commandBuffer)
{
    if (m_viewportCamera == nullptr)
        m_viewportCamera = &m_mainCamera;
    if (IsPlaying())
        return;
    m_viewportCamera->Control();
    m_viewportCamera->Update(commandBuffer);
}

void Scene::UpdatePlayCamera(const vk::CommandBuffer& commandBuffer)
{
    if (m_playCamera == nullptr)
        m_playCamera = &m_mainCamera;
    if (!IsPlaying())
        return;
    m_playCamera->Control();
    m_playCamera->Update(commandBuffer);
}

void Scene::UpdatePointLight(const vk::CommandBuffer& commandBuffer)
{
    m_pointLight.Update(commandBuffer);
    for (int i = 0; i < m_pointLight.Size(); i++) {
        m_shadowCubemaps[i]->DrawShadowMap(i, m_pointLight, m_meshes, commandBuffer);
    }
}

void Scene::UpdatePointLightBuffer()
{
    m_pointLight.UpdateBuffer();
    UpdateShadowCubemaps();
}

void Scene::UpdateShadowMap(const vk::CommandBuffer& commandBuffer)
{
    Camera* camera;
    if (!IsPlaying())
        camera = m_viewportCamera;
    else
        camera = m_playCamera;
    m_cascadedShadowMap.UpdateCascades(camera, m_dirLight);
    m_cascadedShadowMap.UpdateUBO(m_dirLight, commandBuffer);
    m_cascadedShadowMap.Draw(m_meshes, commandBuffer);
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
        physicsDebugPipeline.UpdateMeshUBO(GetSelectedMeshInstance().physicsDebugUBOBuffer->Get().descriptorBufferInfo);
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

void Scene::Clear(const vk::CommandBuffer& commandBuffer)
{
    UnselectAll();
    Stop(commandBuffer);
    m_sceneFilePath.clear();
    m_hdriFilePath.clear();
    m_envMap.reset();
    m_envCubemap.reset();
    m_irradianceCubemap.reset();
    m_prefilteredCubemap.reset();
    m_meshes.clear();
    m_cameras.clear();
    m_resources.clear();
    m_pointLight.Clear();
    m_shadowCubemaps.clear();
    m_albedoTextures.clear();
    m_normalTextures.clear();
    m_metallicTextures.clear();
    m_roughnessTextures.clear();
    Script::s_scriptClasses.clear();
    Script::s_scriptInstances.clear();
    m_playCamera = &m_mainCamera;
    m_mainCamera.SetInitPos();
    m_dirLight.Init();
    m_showLightIcon = true;
    m_showGrid = true;
    m_iblExposure = 1.0f;
    postProcessPushConstants.useMotionBlur = 0;
    postProcessPushConstants.divisor = 20.0f;
}

void Scene::CopyMeshInstances()
{
    m_meshCopies.reserve(m_meshes.size());
    for (auto& mesh : m_meshes) {
        m_meshCopies.emplace_back(mesh->m_meshColorID);
        m_meshCopies.back().m_meshInstances.reserve(mesh->m_meshInstances.size());
        for (auto& meshInstance : mesh->m_meshInstances) {
            m_meshCopies.back().AddInstance(meshInstance->UUID);
            *m_meshCopies.back().m_meshInstances.back() = *meshInstance;
            if (meshInstance->physicsInfo)
                m_meshCopies.back().m_meshInstances.back()->physicsInfo = std::make_unique<PhysicsInfo>(*meshInstance->physicsInfo);
            if (meshInstance->camera)
                m_meshCopies.back().m_meshInstances.back()->camera = std::make_unique<SubCamera>(meshInstance->UUID);
        }
    }
}

void Scene::RevertMeshInstances(const vk::CommandBuffer& commandBuffer)
{
    if (m_meshCopies.empty())
        return;
    uint64_t playCameraID = m_playCamera->GetID();
    for (int i = 0; i < m_meshes.size(); i++) {
        m_meshes[i]->m_meshInstances.clear();
        for (int j = 0; j < m_meshCopies[i].m_meshInstances.size(); j++) {
            AddMeshInstance(*m_meshes[i], m_meshCopies[i].m_meshInstances[j]->UUID);
            auto& meshInstance = m_meshes[i]->m_meshInstances.back();
            auto& meshInstanceCopy = m_meshCopies[i].m_meshInstances[j];
            *meshInstance = *meshInstanceCopy;
            if (meshInstanceCopy->physicsInfo) {
                AddPhysics(commandBuffer, *m_meshes[i], *meshInstance, *meshInstanceCopy->physicsInfo);
                meshInstance->physicsDebugUBO.scale = meshInstanceCopy->physicsInfo->scale;
            }
            if (meshInstanceCopy->camera) {
                AddCamera(*meshInstance);
                if (playCameraID == meshInstance->UUID)
                    m_playCamera = meshInstance->camera.get();
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
        Script::Reset();
        for (auto& scriptClass : Script::s_scriptClasses)
            scriptClass.second->Init();
        for (auto& scriptInstance : Script::s_scriptInstances) {
            scriptInstance.second->Init();
            scriptInstance.second->InvokeOnCreate();
        }
        UpdateCameraDescriptor(m_playCamera);
    }

    Physics::Simulate(m_meshes);

    for (auto& scriptInstance : Script::s_scriptInstances) {
        scriptInstance.second->InvokeOnUpdate(Time::GetDeltaTime());
    }
}

void Scene::Stop(const vk::CommandBuffer& commandBuffer)
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
    RevertMeshInstances(commandBuffer);
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
    for (auto& imageLoadCommandPool : m_imageLoadCommandPools)
        vkn::Device::Get().device.destroyCommandPool(imageLoadCommandPool);
}

void Scene::AddCamera(MeshInstance& instance)
{
    instance.camera = std::make_unique<SubCamera>(instance.UUID);
    m_cameras[instance.UUID] = instance.camera.get();
    instance.camera->GetTranslation() = instance.translation;
    instance.camera->Reset();
}

void Scene::DeleteMesh(int index)
{
    for (int i = 0; i < m_meshes[index]->m_meshInstances.size();)
        DeleteMeshInstance(*m_meshes[index], *m_meshes[index]->m_meshInstances[i]);
    DeleteMatrials(index);
    m_meshes.erase(m_meshes.begin() + index);
    for (int32_t i = index; i < m_meshes.size(); i++) {
        m_meshes[index]->m_meshColorID--;
        m_meshes[index]->UpdateColorID();
    }
}

void Scene::DeleteCamera(const uint64_t ID)
{
    if (m_playCamera->GetID() == ID)
        m_playCamera = &m_mainCamera;
    m_cameras.erase(ID);
}

void Scene::DeleteMatrials(int index)
{
    uint32_t pos = 0;
    for (int i = 0; i < index; i++) {
        pos += m_meshes[i]->m_materials.size();
    }
    for (int i = 0; i < m_meshes[index]->m_materials.size(); i++) {
        m_albedoTextures.erase(m_albedoTextures.begin() + pos);
        m_roughnessTextures.erase(m_roughnessTextures.begin() + pos);
        m_metallicTextures.erase(m_metallicTextures.begin() + pos);
        m_normalTextures.erase(m_normalTextures.begin() + pos);
    }
    UpdateTextureDescriptors();
}
