#include "viewport.h"

Viewport::Viewport() : m_panelRatio(0.0f), m_outDated(false), m_isMouseHovered(false)
{
    m_extent = vk::Extent2D((uint32_t)vkn::Swapchain::Get().swapchainImageExtent.width, (uint32_t)vkn::Swapchain::Get().swapchainImageExtent.height);

    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffers);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_pickColorCommandBuffers);

    meshRenderPipeline.CreatePipeline();
    shadowMapPipeline.CreatePipeline();
    shadowCubemapPipeline.CreatePipeline();
    envCubemapPipeline.CreatePipeline();
    irradianceCubemapPipeline.CreatePipeline();
    prefilteredCubemapPipeline.CreatePipeline();
    brdfLutPipeline.CreatePipeline();
    skyboxRenderPipeline.CreatePipeline();
    lineRenderPipeline.CreatePipeline();

    CreateImage();

    m_pickedColor.CreateImage({ 1, 1, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eTransferDst, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void Viewport::CreateImage()
{
    m_image.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_image.CreateImageView();

    m_colorID.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_colorID.CreateImageView();

    m_depth.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_depth.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_depth.CreateImageView();

    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_image.Get().image,
                                        {},
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        {},
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTopOfPipe,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_colorID.Get().image,
                                        {},
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        {},
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTopOfPipe,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();
    vkn::Command::SubmitAndWait(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);

    std::vector<vk::ImageView> attachments = {
        m_image.Get().imageView,
        m_colorID.Get().imageView,
        m_depth.Get().imageView,
    };
    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = meshRenderPipeline.m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_extent.width;
    framebufferInfo.height = m_extent.height;
    framebufferInfo.layers = 1;

    m_framebuffer = vkn::Device::Get().device.createFramebuffer(framebufferInfo);
}

void Viewport::DestroyImage()
{
    vkn::Device::Get().device.destroyFramebuffer(m_framebuffer);

    m_image.DestroyImage();
    m_image.DestroyImageView();
    m_image.m_memory.Free();

    m_colorID.DestroyImage();
    m_colorID.DestroyImageView();
    m_colorID.m_memory.Free();

    m_depth.DestroyImage();
    m_depth.DestroyImageView();
    m_depth.m_memory.Free();
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
    vkn::Command::Begin(m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::SetImageMemoryBarrier(m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_colorID.Get().image,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::ImageLayout::eTransferSrcOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eTransferRead,
                                        vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::PipelineStageFlagBits::eTransfer);
    vkn::Command::SetImageMemoryBarrier(m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_pickedColor.Get().image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        {},
                                        vk::AccessFlagBits::eTransferWrite,
                                        vk::PipelineStageFlagBits::eTransfer,
                                        vk::PipelineStageFlagBits::eTransfer);

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

    m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()].copyImage(m_colorID.Get().image, vk::ImageLayout::eTransferSrcOptimal, m_pickedColor.Get().image, vk::ImageLayout::eTransferDstOptimal, region);

    vkn::Command::SetImageMemoryBarrier(m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_colorID.Get().image,
                                        vk::ImageLayout::eTransferSrcOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eTransferRead,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTransfer,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_pickedColor.Get().image,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::ImageLayout::eGeneral,
                                        vk::AccessFlagBits::eTransferRead,
                                        vk::AccessFlagBits::eMemoryRead,
                                        vk::PipelineStageFlagBits::eTransfer,
                                        vk::PipelineStageFlagBits::eTransfer);
    m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();

    vkn::Command::SubmitAndWait(m_pickColorCommandBuffers[vkn::Sync::GetCurrentFrameIndex()]);

    const int32_t* data;
    vkn::CheckResult(vkn::Device::Get().device.mapMemory(m_pickedColor.m_memory.GetMemory(), 0, vk::WholeSize, {}, (void**)&data));
    std::cout << "mesh id: " << data[0] << " instance id: " << data[1] << '\n';
    scene.SelectByColorID(data[0], data[1]);
    vkn::Device::Get().device.unmapMemory(m_pickedColor.m_memory.GetMemory());
}

void Viewport::Draw(const Scene& scene)
{
    vkn::Command::Begin(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_image.Get().image,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_colorID.Get().image,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = meshRenderPipeline.m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = m_extent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue colorClearValue;
    vk::ClearValue idClearValue;
    colorClearValue.color = { std::array<float, 4>{ 0.05f, 0.05f, 0.05f, 1.0f } };
    idClearValue.color = { std::array<int32_t, 4>{ -1, -1, -1, -1 } };
    vk::ClearValue depthClearValue;
    depthClearValue.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 3;
    vk::ClearValue clearValues[] = { colorClearValue, idClearValue, depthClearValue };
    renderPassInfo.pClearValues = clearValues;

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height);
    viewport.width = static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.width);
    viewport.height = -1.0f * static_cast<float>(vkn::Swapchain::Get().swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = vkn::Swapchain::Get().swapchainImageExtent;

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].setViewport(0, viewport);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].setScissor(0, scissor);

    vk::DeviceSize vertexOffsets[]{ 0 };

    {
        // skybox
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.m_pipelineLayout, 0, 1, &skyboxRenderPipeline.m_descriptorSets[0], 0, nullptr);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.m_pipeline);
        skyboxRenderPushConstants.exposure = scene.m_iblExposure;
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].pushConstants(
            skyboxRenderPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(SkyboxRenderPushConstants),
            &skyboxRenderPushConstants);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindVertexBuffers(0, 1, &scene.m_envCube.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindIndexBuffer(scene.m_envCube.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
        m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].drawIndexed(scene.m_envCube.GetIndicesCount(0), scene.m_envCube.GetInstanceCount(), 0, 0, 0);
    }

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindPipeline(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.m_pipeline);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.m_pipelineLayout, 0, meshRenderPipeline.m_descriptorSets.size(), meshRenderPipeline.m_descriptorSets.data(), 0, nullptr);

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
                m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
                m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
                meshRenderPushConsts.materialID = materialOffset + part.materialID;
                m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].pushConstants(
                    meshRenderPipeline.m_pipelineLayout,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(MeshRenderPushConstants),
                    &meshRenderPushConsts);
                m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
            }
        }
        materialOffset += (int)mesh->GetMaterialCount();
    }
    // physics debug
    if (scene.m_selectedMeshID > -1 && scene.m_selectedMeshInstanceID > -1) {
        if (scene.GetSelectedMeshInstance().physicsInfo) {
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindPipeline(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.m_pipeline);
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.m_pipelineLayout, 0, lineRenderPipeline.m_descriptorSets.size(), lineRenderPipeline.m_descriptorSets.data(), 0, nullptr);
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindVertexBuffers(0, 1, &scene.GetSelectedMeshInstance().physicsDebugDrawer->m_vertexBuffer->Get().buffer, vertexOffsets);
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].bindIndexBuffer(scene.GetSelectedMeshInstance().physicsDebugDrawer->m_indexBuffer->Get().buffer, 0, vk::IndexType::eUint32);
            m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].drawIndexed(scene.GetSelectedMeshInstance().physicsDebugDrawer->m_lineIndices.size(), 1, 0, 0, 0);
        }
    }

    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].endRenderPass();

    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_image.Get().image,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()],
                                        m_colorID.Get().image,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()].end();

    // vkn::Device::s_submitInfos.emplace_back(0, nullptr, nullptr, 1, &m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
    vkn::Command::SubmitAndWait(m_commandBuffers[vkn::Sync::GetCurrentFrameIndex()]);
}

Viewport::~Viewport()
{
    vkn::Device::Get().device.destroyFramebuffer(m_framebuffer);
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    vkn::Device::Get().device.destroySampler(vkn::Image::s_repeatSampler);
    vkn::Device::Get().device.destroySampler(vkn::Image::s_clampSampler);
    meshRenderPipeline.Destroy();
    shadowMapPipeline.Destroy();
    shadowCubemapPipeline.Destroy();
    envCubemapPipeline.Destroy();
    irradianceCubemapPipeline.Destroy();
    prefilteredCubemapPipeline.Destroy();
    brdfLutPipeline.Destroy();
    skyboxRenderPipeline.Destroy();
    lineRenderPipeline.Destroy();
}
