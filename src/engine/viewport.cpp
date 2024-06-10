#include "viewport.h"

Viewport::Viewport() : m_panelRatio(0.0f), m_outDated(false), m_isMouseHovered(false)
{
    m_extent = vk::Extent2D((uint32_t)vkn::Swapchain::Get().swapchainImageExtent.width, (uint32_t)vkn::Swapchain::Get().swapchainImageExtent.height);

    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);

    meshRenderPipeline.CreatePipeline();
    colorIDPipeline.CreatePipeline();
    shadowMapPipeline.CreatePipeline();
    shadowCubemapPipeline.CreatePipeline();
    envCubemapPipeline.CreatePipeline();
    irradianceCubemapPipeline.CreatePipeline();
    prefilteredCubemapPipeline.CreatePipeline();
    brdfLutPipeline.CreatePipeline();
    skyboxRenderPipeline.CreatePipeline();
    lineRenderPipeline.CreatePipeline();

    m_pickedColor.CreateImage({ 1, 1, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eTransferDst, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void Viewport::CreateImage()
{
    m_image.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_repeatSampler, 1, vkn::Device::Get().maxSampleCount);
    m_image.CreateImageView();

    m_depth.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_repeatSampler, 1, vkn::Device::Get().maxSampleCount);
    m_depth.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_depth.CreateImageView();

    m_final.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_final.CreateImageView();

    vkn::Command::Begin(m_commandBuffer);
    vkn::Command::ChangeImageLayout(m_commandBuffer, m_final.Get().image, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);
    m_commandBuffer.end();
    vkn::Command::SubmitAndWait(m_commandBuffer);

    std::vector<vk::ImageView> attachments = {
        m_image.Get().imageView,
        m_final.Get().imageView,
        m_depth.Get().imageView
    };
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = meshRenderPipeline.m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    m_framebuffer = vkn::Device::Get().device.createFramebuffer(framebufferInfo);

    m_colorID.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_colorID.CreateImageView();

    m_colorIDDepth.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_colorIDDepth.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_colorIDDepth.CreateImageView();

    attachments = {
        m_colorID.Get().imageView,
        m_colorIDDepth.Get().imageView,
    };
    framebufferInfo.renderPass = colorIDPipeline.m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    m_colorIDFramebuffer = vkn::Device::Get().device.createFramebuffer(framebufferInfo);
}

void Viewport::DestroyImage()
{
    vkn::Device::Get().device.destroyFramebuffer(m_framebuffer);
    vkn::Device::Get().device.destroyFramebuffer(m_colorIDFramebuffer);

    m_image.DestroyImage();
    m_image.DestroyImageView();
    m_image.m_memory.Free();

    m_final.DestroyImage();
    m_final.DestroyImageView();
    m_final.m_memory.Free();

    m_depth.DestroyImage();
    m_depth.DestroyImageView();
    m_depth.m_memory.Free();

    m_colorID.DestroyImage();
    m_colorID.DestroyImageView();
    m_colorID.m_memory.Free();

    m_colorIDDepth.DestroyImage();
    m_colorIDDepth.DestroyImageView();
    m_colorIDDepth.m_memory.Free();
}

void Viewport::UpdateImage()
{
    m_panelRatio = m_panelSize.x / m_panelSize.y;

    m_extent.width = (uint32_t)(m_panelSize.x);
    m_extent.height = (uint32_t)(m_panelSize.y);
#if defined(__APPLE__)
    m_extent.width = (uint32_t)(m_panelSize.x * ImGui::GetWindowDpiScale());
    m_extent.height = (uint32_t)(m_panelSize.y * ImGui::GetWindowDpiScale());
#endif

    if (m_extent.width == 0 || m_extent.height == 0)
        m_extent = vkn::Swapchain::Get().swapchainImageExtent;

    DestroyImage();
    CreateImage();

    m_outDated = false;
}

void Viewport::PickColor(double mouseX, double mouseY, Scene& scene)
{
    vkn::Command::Begin(m_commandBuffer);
    vkn::Command::ChangeImageLayout(m_commandBuffer, m_colorID.Get().image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);
    vkn::Command::ChangeImageLayout(m_commandBuffer, m_pickedColor.Get().image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    vk::ImageCopy region;
    region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.srcSubresource.layerCount = 1;
    region.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.dstSubresource.layerCount = 1;
    float scaleX, scaleY;
    glfwGetWindowContentScale(Window::GetWindow(), &scaleX, &scaleY);
    int32_t offsetX = 0, offsetY = 0;
    if (mouseX > m_panelPos.x)
        offsetX = (int32_t)((mouseX - m_panelPos.x));
    if (mouseY > m_panelPos.y)
        offsetY = (int32_t)((mouseY - m_panelPos.y));
        // TODO:
#if defined(__APPLE__)
    if (mouseX > m_panelPos.x)
        offsetX = (int32_t)((mouseX - m_panelPos.x) * scaleX);
    if (mouseY > m_panelPos.y)
        offsetY = (int32_t)((mouseY - m_panelPos.y) * scaleY);
#endif
    region.srcOffset.x = offsetX;
    region.srcOffset.y = offsetY;
    region.extent.width = 1;
    region.extent.height = 1;
    region.extent.depth = 1;

    m_commandBuffer.copyImage(m_colorID.Get().image, vk::ImageLayout::eTransferSrcOptimal, m_pickedColor.Get().image, vk::ImageLayout::eTransferDstOptimal, region);

    vkn::Command::ChangeImageLayout(m_commandBuffer, m_colorID.Get().image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    vkn::Command::ChangeImageLayout(m_commandBuffer, m_pickedColor.Get().image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral);
    m_commandBuffer.end();

    vkn::Command::SubmitAndWait(m_commandBuffer);

    const int32_t* data;
    vkn::CheckResult(vkn::Device::Get().device.mapMemory(m_pickedColor.m_memory.GetMemory(), 0, vk::WholeSize, {}, (void**)&data));
    std::cout << "mesh id: " << data[0] << " instance id: " << data[1] << '\n';
    scene.SelectByColorID(data[0], data[1]);
    vkn::Device::Get().device.unmapMemory(m_pickedColor.m_memory.GetMemory());
}

void Viewport::Draw(const Scene& scene, const vk::CommandBuffer& commandBuffer)
{
    vk::ClearValue clearValue;
    clearValue.color = { std::array<float, 4>{ 0.05f, 0.05f, 0.05f, 1.0f } };
    clearValue.depthStencil.depth = 1.0f;
    std::vector<vk::ClearValue> clearValues = { clearValue, clearValue, clearValue };
    vkn::Command::BeginRenderPass(commandBuffer, meshRenderPipeline.m_renderPass, m_framebuffer, { {}, m_extent }, clearValues);
    vkn::Command::SetViewport(commandBuffer);
    vk::DeviceSize vertexOffsets[]{ 0 };
    {
        // Skybox
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.m_pipelineLayout, 0, 1, &skyboxRenderPipeline.m_descriptorSets[0], 0, nullptr);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.m_pipeline);
        skyboxRenderPushConstants.exposure = scene.m_iblExposure;
        commandBuffer.pushConstants(
            skyboxRenderPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(SkyboxRenderPushConstants),
            &skyboxRenderPushConstants);
        commandBuffer.bindVertexBuffers(0, 1, &scene.m_envCube.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
        commandBuffer.bindIndexBuffer(scene.m_envCube.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
        commandBuffer.drawIndexed(scene.m_envCube.GetIndicesCount(0), scene.m_envCube.GetInstanceCount(), 0, 0, 0);
    }

    {
        // Mesh
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.m_pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.m_pipelineLayout, 0, meshRenderPipeline.m_descriptorSets.size(), meshRenderPipeline.m_descriptorSets.data(), 0, nullptr);
        meshRenderPushConsts.iblExposure = scene.m_iblExposure;
        // not the actual index; doing this being unable to bind instance UBO in case there is no instance
        int meshIndex = 0;
        int materialOffset = 0;
        for (const auto& mesh : scene.m_meshes) {
            if (mesh->GetInstanceCount() > 0) {
                meshRenderPushConsts.meshIndex = meshIndex;
                meshRenderPushConsts.lightCount = (int)scene.m_pointLight.Size();
                meshIndex++;
                for (const auto& part : mesh->GetMeshParts()) {
                    commandBuffer.bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
                    commandBuffer.bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
                    meshRenderPushConsts.materialID = materialOffset + part.materialID;
                    commandBuffer.pushConstants(
                        meshRenderPipeline.m_pipelineLayout,
                        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                        0,
                        sizeof(MeshRenderPushConstants),
                        &meshRenderPushConsts);
                    commandBuffer.drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
                }
            }
            materialOffset += (int)mesh->GetMaterialCount();
        }
    }

    // Physics Debug
    if (scene.m_selectedMeshID > -1 && scene.m_selectedMeshInstanceID > -1) {
        if (scene.GetSelectedMeshInstance().physicsInfo) {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.m_pipeline);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.m_pipelineLayout, 0, lineRenderPipeline.m_descriptorSets.size(), lineRenderPipeline.m_descriptorSets.data(), 0, nullptr);
            commandBuffer.bindVertexBuffers(0, 1, &scene.GetSelectedMeshInstance().physicsDebugDrawer->m_vertexBuffer->Get().buffer, vertexOffsets);
            commandBuffer.bindIndexBuffer(scene.GetSelectedMeshInstance().physicsDebugDrawer->m_indexBuffer->Get().buffer, 0, vk::IndexType::eUint32);
            commandBuffer.drawIndexed(scene.GetSelectedMeshInstance().physicsDebugDrawer->m_lineIndices.size(), 1, 0, 0, 0);
        }
    }
    commandBuffer.endRenderPass();

    // Color ID
    clearValue.color = { std::array<int32_t, 4>{ -1, -1, -1, -1 } };
    clearValue.depthStencil.depth = 1.0f;
    clearValues = { clearValue, clearValue };
    vkn::Command::BeginRenderPass(commandBuffer, colorIDPipeline.m_renderPass, m_colorIDFramebuffer, { {}, m_extent }, clearValues);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, colorIDPipeline.m_pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, colorIDPipeline.m_pipelineLayout, 0, colorIDPipeline.m_descriptorSets.size(), colorIDPipeline.m_descriptorSets.data(), 0, nullptr);

    int meshIndex = 0;
    for (const auto& mesh : scene.m_meshes) {
        if (mesh->GetInstanceCount() > 0) {
            meshRenderPushConsts.meshIndex = meshIndex;
            meshIndex++;
            for (const auto& part : mesh->GetMeshParts()) {
                commandBuffer.bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
                commandBuffer.bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
                commandBuffer.pushConstants(
                    meshRenderPipeline.m_pipelineLayout,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(MeshRenderPushConstants),
                    &meshRenderPushConsts);
                commandBuffer.drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
            }
        }
    }
    commandBuffer.endRenderPass();
}

Viewport::~Viewport()
{
    vkn::Device::Get().device.destroyFramebuffer(m_framebuffer);
    vkn::Device::Get().device.destroyFramebuffer(m_colorIDFramebuffer);
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    vkn::Device::Get().device.destroySampler(vkn::Image::s_repeatSampler);
    vkn::Device::Get().device.destroySampler(vkn::Image::s_clampSampler);
    meshRenderPipeline.Destroy();
    colorIDPipeline.Destroy();
    shadowMapPipeline.Destroy();
    shadowCubemapPipeline.Destroy();
    envCubemapPipeline.Destroy();
    irradianceCubemapPipeline.Destroy();
    prefilteredCubemapPipeline.Destroy();
    brdfLutPipeline.Destroy();
    skyboxRenderPipeline.Destroy();
    lineRenderPipeline.Destroy();
}
