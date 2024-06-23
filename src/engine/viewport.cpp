#include "viewport.h"

Viewport::Viewport() : m_outDated(false), m_isMouseHovered(false)
{
    m_extent = vk::Extent2D((uint32_t)vkn::Swapchain::Get().swapchainImageExtent.width, (uint32_t)vkn::Swapchain::Get().swapchainImageExtent.height);

    vkn::Command::CreateCommandPool(m_commandPool);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_commandBuffer);
    vkn::Command::AllocateCommandBuffer(m_commandPool, m_pickColorCommandBuffer);

    meshRenderPipeline.CreatePipeline();
    postProcessPipeline.CreatePipeline();
    colorIDPipeline.CreatePipeline();
    shadowMapPipeline.CreatePipeline();
    shadowCubemapPipeline.CreatePipeline();
    envCubemapPipeline.CreatePipeline();
    irradianceCubemapPipeline.CreatePipeline();
    prefilteredCubemapPipeline.CreatePipeline();
    brdfLutPipeline.CreatePipeline();
    skyboxRenderPipeline.CreatePipeline();
    lineRenderPipeline.CreatePipeline();
    physicsDebugPipeline.CreatePipeline();

    m_pickedColor.CreateImage({ 1, 1, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eTransferDst, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void Viewport::CreateImage()
{
    m_imageSampled.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_repeatSampler, 1, vkn::Device::Get().maxSampleCount);
    m_imageSampled.CreateImageView();

    m_depthSampled.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, vkn::Image::s_repeatSampler, 1, vkn::Device::Get().maxSampleCount);
    m_depthSampled.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_depthSampled.CreateImageView();

    m_imageResolved.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_imageResolved.CreateImageView();
    vk::DescriptorImageInfo imageInfo(vkn::Image::s_clampSampler, m_imageResolved.Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
    postProcessPipeline.UpdateRenderImage(imageInfo);

    m_imageFinal.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_imageFinal.CreateImageView();

    std::vector<vk::ImageView>
        attachments = {
            m_imageSampled.Get().imageView,
            m_imageResolved.Get().imageView,
            m_depthSampled.Get().imageView,
            m_imageFinal.Get().imageView
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

    m_depth.CreateImage({ m_extent.width, m_extent.height, 1 }, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    m_depth.m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    m_depth.CreateImageView();
    imageInfo = { vkn::Image::s_repeatSampler, m_depth.Get().imageView, vk::ImageLayout::eShaderReadOnlyOptimal };
    postProcessPipeline.UpdateDepthMap(imageInfo);

    attachments = {
        m_colorID.Get().imageView,
        m_depth.Get().imageView,
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

    m_imageSampled.DestroyImage();
    m_imageSampled.DestroyImageView();
    m_imageSampled.m_memory.Free();

    m_imageResolved.DestroyImage();
    m_imageResolved.DestroyImageView();
    m_imageResolved.m_memory.Free();

    m_depthSampled.DestroyImage();
    m_depthSampled.DestroyImageView();
    m_depthSampled.m_memory.Free();

    m_colorID.DestroyImage();
    m_colorID.DestroyImageView();
    m_colorID.m_memory.Free();

    m_depth.DestroyImage();
    m_depth.DestroyImageView();
    m_depth.m_memory.Free();

    m_imageFinal.DestroyImage();
    m_imageFinal.DestroyImageView();
    m_imageFinal.m_memory.Free();
}

void Viewport::UpdateImage()
{
    s_panelRatio = m_panelSize.x / m_panelSize.y;

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
    vkn::Command::Begin(m_pickColorCommandBuffer);
    vkn::Command::ChangeImageLayout(m_pickColorCommandBuffer, m_colorID.Get().image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eTransferSrcOptimal);
    vkn::Command::ChangeImageLayout(m_pickColorCommandBuffer, m_pickedColor.Get().image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

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

    m_pickColorCommandBuffer.copyImage(m_colorID.Get().image, vk::ImageLayout::eTransferSrcOptimal, m_pickedColor.Get().image, vk::ImageLayout::eTransferDstOptimal, region);

    vkn::Command::ChangeImageLayout(m_pickColorCommandBuffer, m_colorID.Get().image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    vkn::Command::ChangeImageLayout(m_pickColorCommandBuffer, m_pickedColor.Get().image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral);
    vkn::Command::End(m_pickColorCommandBuffer);
    vkn::Command::SubmitAndWait(m_pickColorCommandBuffer);

    const int32_t* data;
    vkn::CheckResult(vkn::Device::Get().device.mapMemory(m_pickedColor.m_memory.GetMemory(), 0, vk::WholeSize, {}, (void**)&data));
    Log(DEBUG, fmt::terminal_color::bright_black, "[Mesh ID: {0}, Instance ID: {1}]", data[0], data[1]);
    scene.SelectByColorID(data[0], data[1]);
    vkn::Device::Get().device.unmapMemory(m_pickedColor.m_memory.GetMemory());
}

void Viewport::Draw(const Scene& scene)
{
    vkn::Command::BeginRenderPass(m_commandBuffer, meshRenderPipeline.m_renderPass, m_framebuffer, { {}, m_extent }, meshRenderPipeline.m_clearValues);
    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = (float)m_extent.height;
    viewport.width = (float)m_extent.width;
    viewport.height = -1.0f * (float)m_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = m_extent;

    m_commandBuffer.setViewport(0, viewport);
    m_commandBuffer.setScissor(0, scissor);
    vk::DeviceSize vertexOffsets[]{ 0 };
    {
        // Skybox
        m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.m_pipelineLayout, 0, 1, &skyboxRenderPipeline.m_descriptorSets[0], 0, nullptr);
        m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.m_pipeline);
        skyboxRenderPushConstants.exposure = scene.m_iblExposure;
        m_commandBuffer.pushConstants(
            skyboxRenderPipeline.m_pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(SkyboxRenderPushConstants),
            &skyboxRenderPushConstants);
        m_commandBuffer.bindVertexBuffers(0, 1, &scene.m_cube.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
        m_commandBuffer.bindIndexBuffer(scene.m_cube.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
        m_commandBuffer.drawIndexed(scene.m_cube.GetIndicesCount(0), scene.m_cube.GetInstanceCount(), 0, 0, 0);
    }

    {
        // Mesh
        m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.m_pipeline);
        m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.m_pipelineLayout, 0, meshRenderPipeline.m_descriptorSets.size(), meshRenderPipeline.m_descriptorSets.data(), 0, nullptr);
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
                    m_commandBuffer.bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
                    m_commandBuffer.bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
                    meshRenderPushConsts.materialID = materialOffset + part.materialID;
                    m_commandBuffer.pushConstants(meshRenderPipeline.m_pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(MeshRenderPushConstants), &meshRenderPushConsts);
                    m_commandBuffer.drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
                }
            }
            materialOffset += (int)mesh->GetMaterialCount();
        }
    }

    // Grid
    if (scene.m_showGrid && !scene.IsPlaying()) {
        m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.m_pipeline);
        m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.m_pipelineLayout, 0, lineRenderPipeline.m_descriptorSets.size(), lineRenderPipeline.m_descriptorSets.data(), 0, nullptr);
        m_commandBuffer.bindVertexBuffers(0, 1, &scene.m_grid.m_vertexBuffer->Get().buffer, vertexOffsets);
        m_commandBuffer.bindIndexBuffer(scene.m_grid.m_indexBuffer->Get().buffer, 0, vk::IndexType::eUint32);
        m_commandBuffer.drawIndexed(scene.m_grid.m_lineIndices.size(), 1, 0, 0, 0);
    }

    // Physics Debug
    if (scene.m_selectedMeshID > -1 && scene.m_selectedMeshInstanceID > -1) {
        if (scene.GetSelectedMeshInstance().physicsInfo) {
            m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, physicsDebugPipeline.m_pipeline);
            m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, physicsDebugPipeline.m_pipelineLayout, 0, physicsDebugPipeline.m_descriptorSets.size(), physicsDebugPipeline.m_descriptorSets.data(), 0, nullptr);
            m_commandBuffer.bindVertexBuffers(0, 1, &scene.GetSelectedMeshInstance().physicsDebugDrawer->m_vertexBuffer->Get().buffer, vertexOffsets);
            m_commandBuffer.bindIndexBuffer(scene.GetSelectedMeshInstance().physicsDebugDrawer->m_indexBuffer->Get().buffer, 0, vk::IndexType::eUint32);
            m_commandBuffer.drawIndexed(scene.GetSelectedMeshInstance().physicsDebugDrawer->m_lineIndices.size(), 1, 0, 0, 0);
        }
    }

    // Post Process
    m_commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    m_commandBuffer.pushConstants(postProcessPipeline.m_pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(PostProcessPushConstants), &postProcessPushConstants);
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, postProcessPipeline.m_pipeline);
    m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, postProcessPipeline.m_pipelineLayout, 0, postProcessPipeline.m_descriptorSets.size(), postProcessPipeline.m_descriptorSets.data(), 0, nullptr);
    m_commandBuffer.bindVertexBuffers(0, 1, &scene.m_square.m_vertexBuffers[0]->Get().buffer, vertexOffsets);
    m_commandBuffer.bindIndexBuffer(scene.m_square.m_indexBuffers[0]->Get().buffer, 0, vk::IndexType::eUint32);
    m_commandBuffer.drawIndexed(scene.m_square.GetIndicesCount(0), scene.m_square.GetInstanceCount(), 0, 0, 0);
    m_commandBuffer.endRenderPass();

    // Color ID
    vkn::Command::BeginRenderPass(m_commandBuffer, colorIDPipeline.m_renderPass, m_colorIDFramebuffer, { {}, m_extent }, colorIDPipeline.m_clearValues);
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, colorIDPipeline.m_pipeline);
    m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, colorIDPipeline.m_pipelineLayout, 0, colorIDPipeline.m_descriptorSets.size(), colorIDPipeline.m_descriptorSets.data(), 0, nullptr);

    int meshIndex = 0;
    for (const auto& mesh : scene.m_meshes) {
        if (mesh->GetInstanceCount() > 0) {
            meshRenderPushConsts.meshIndex = meshIndex;
            meshIndex++;
            for (const auto& part : mesh->GetMeshParts()) {
                m_commandBuffer.bindVertexBuffers(0, 1, &mesh->m_vertexBuffers[part.bufferIndex]->Get().buffer, vertexOffsets);
                m_commandBuffer.bindIndexBuffer(mesh->m_indexBuffers[part.bufferIndex]->Get().buffer, 0, vk::IndexType::eUint32);
                m_commandBuffer.pushConstants(
                    meshRenderPipeline.m_pipelineLayout,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(MeshRenderPushConstants),
                    &meshRenderPushConsts);
                m_commandBuffer.drawIndexed(mesh->GetIndicesCount(part.bufferIndex), mesh->GetInstanceCount(), 0, 0, 0);
            }
        }
    }
    m_commandBuffer.endRenderPass();
}

Viewport::~Viewport()
{
    vkn::Device::Get().device.destroyFramebuffer(m_framebuffer);
    vkn::Device::Get().device.destroyFramebuffer(m_colorIDFramebuffer);
    vkn::Device::Get().device.destroyCommandPool(m_commandPool);
    vkn::Device::Get().device.destroySampler(vkn::Image::s_repeatSampler);
    vkn::Device::Get().device.destroySampler(vkn::Image::s_clampSampler);
    meshRenderPipeline.Destroy();
    postProcessPipeline.Destroy();
    colorIDPipeline.Destroy();
    shadowMapPipeline.Destroy();
    shadowCubemapPipeline.Destroy();
    envCubemapPipeline.Destroy();
    irradianceCubemapPipeline.Destroy();
    prefilteredCubemapPipeline.Destroy();
    brdfLutPipeline.Destroy();
    skyboxRenderPipeline.Destroy();
    lineRenderPipeline.Destroy();
    physicsDebugPipeline.Destroy();
}
