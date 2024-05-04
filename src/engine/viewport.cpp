#include "viewport.h"

Viewport::Viewport() : panelRatio(0.0f), outDated(false), isMouseHovered(false)
{
    extent = vk::Extent2D((uint32_t)vkn::Swapchain::GetBundle().swapchainImageExtent.width, (uint32_t)vkn::Swapchain::GetBundle().swapchainImageExtent.height);

    vkn::Command::CreateCommandPool(commandPool_);
    vkn::Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    meshRenderPipeline.CreatePipeline();
    shadowMapPipeline.CreatePipeline();
    shadowCubemapPipeline.CreatePipeline();
    envCubemapPipeline.CreatePipeline();
    irradianceCubemapPipeline.CreatePipeline();
    prefilteredCubemapPipeline.CreatePipeline();
    brdfLutPipeline.CreatePipeline();
    skyboxRenderPipeline.CreatePipeline();
    lineRenderPipeline.CreatePipeline();

    CreateViewportImages();
    CreateViewportFrameBuffer();

    colorPicked_.CreateImage({ 1, 1, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eTransferDst, vk::ImageTiling::eLinear, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void Viewport::CreateViewportImages()
{
    viewportImage.CreateImage({ extent.width, extent.height, 1 }, vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    viewportImage.CreateImageView();

    colorID.CreateImage({ extent.width, extent.height, 1 }, vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    colorID.CreateImageView();

    depthImage.CreateImage({ extent.width, extent.height, 1 }, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    depthImage.imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthImage.CreateImageView();

    vkn::Command::Begin(commandBuffer_);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        viewportImage.GetBundle().image,
                                        {},
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        {},
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTopOfPipe,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorID.GetBundle().image,
                                        {},
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        {},
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTopOfPipe,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    vkn::Command::Submit(&commandBuffer_, 1);
}

void Viewport::DestroyViewportImages()
{
    viewportImage.DestroyImage();
    viewportImage.DestroyImageView();
    viewportImage.memory.Free();

    colorID.DestroyImage();
    colorID.DestroyImageView();
    colorID.memory.Free();

    depthImage.DestroyImage();
    depthImage.DestroyImageView();
    depthImage.memory.Free();
}

void Viewport::CreateViewportFrameBuffer()
{
    std::vector<vk::ImageView> attachments = {
        viewportImage.GetBundle().imageView,
        colorID.GetBundle().imageView,
        depthImage.GetBundle().imageView,
    };

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = meshRenderPipeline.renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    framebuffer = vkn::Device::GetBundle().device.createFramebuffer(framebufferInfo);
}

const int32_t* Viewport::PickColor(double mouseX, double mouseY)
{
    vkn::Command::Begin(commandBuffer_);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorID.GetBundle().image,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::ImageLayout::eTransferSrcOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eTransferRead,
                                        vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::PipelineStageFlagBits::eTransfer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorPicked_.GetBundle().image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        {},
                                        vk::AccessFlagBits::eTransferWrite,
                                        vk::PipelineStageFlagBits::eTransfer,
                                        vk::PipelineStageFlagBits::eTransfer);
    commandBuffer_.end();
    vkn::Command::Submit(&commandBuffer_, 1);

    vk::ImageCopy region;
    region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.srcSubresource.layerCount = 1;
    region.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.dstSubresource.layerCount = 1;
    float scaleX, scaleY;
    glfwGetWindowContentScale(Window::GetWindow(), &scaleX, &scaleY);
    int32_t offsetX = 0, offsetY = 0;
    if (mouseX > panelPos.x)
        offsetX = (int32_t)((mouseX - panelPos.x));
    if (mouseY > panelPos.y)
        offsetY = (int32_t)((mouseY - panelPos.y));
        // TODO:
#if defined(__APPLE__)
    if (mouseX > panelPos.x)
        offsetX = (int32_t)((mouseX - panelPos.x) * scaleX);
    if (mouseY > panelPos.y)
        offsetY = (int32_t)((mouseY - panelPos.y) * scaleY);
#endif
    region.srcOffset.x = offsetX;
    region.srcOffset.y = offsetY;
    region.extent.width = 1;
    region.extent.height = 1;
    region.extent.depth = 1;

    vkn::Command::Begin(commandBuffer_);
    commandBuffer_.copyImage(colorID.GetBundle().image, vk::ImageLayout::eTransferSrcOptimal, colorPicked_.GetBundle().image, vk::ImageLayout::eTransferDstOptimal, region);

    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorID.GetBundle().image,
                                        vk::ImageLayout::eTransferSrcOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eTransferRead,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eTransfer,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorPicked_.GetBundle().image,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        vk::ImageLayout::eGeneral,
                                        vk::AccessFlagBits::eTransferRead,
                                        vk::AccessFlagBits::eMemoryRead,
                                        vk::PipelineStageFlagBits::eTransfer,
                                        vk::PipelineStageFlagBits::eTransfer);
    commandBuffer_.end();
    vkn::Command::Submit(&commandBuffer_, 1);

    vk::ImageSubresource subResource{ vk::ImageAspectFlagBits::eColor, 0, 0 };
    vk::SubresourceLayout subResourceLayout;
    vkn::Device::GetBundle().device.getImageSubresourceLayout(colorPicked_.GetBundle().image, &subResource, &subResourceLayout);

    const int32_t* data;
    vkn::CheckResult(vkn::Device::GetBundle().device.mapMemory(colorPicked_.memory.GetMemory(), 0, vk::WholeSize, {}, (void**)&data));
    std::cout << "mesh id: " << data[0] << " instance id: " << data[1] << '\n';
    vkn::Device::GetBundle().device.unmapMemory(colorPicked_.memory.GetMemory());

    return data;
}

void Viewport::Draw(const Scene& scene)
{
    vkn::Command::Begin(commandBuffer_);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        viewportImage.GetBundle().image,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorID.GetBundle().image,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::PipelineStageFlagBits::eFragmentShader,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        scene.shadowMap_.GetBundle().image,
                                        vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        { vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = meshRenderPipeline.renderPass;
    renderPassInfo.framebuffer = framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = extent;
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

    commandBuffer_.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(vkn::Swapchain::GetBundle().swapchainImageExtent.height);
    viewport.width = static_cast<float>(vkn::Swapchain::GetBundle().swapchainImageExtent.width);
    viewport.height = -1.0f * static_cast<float>(vkn::Swapchain::GetBundle().swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = vkn::Swapchain::GetBundle().swapchainImageExtent;

    commandBuffer_.setViewport(0, viewport);
    commandBuffer_.setScissor(0, scissor);

    vk::DeviceSize vertexOffsets[]{ 0 };

    {
        // skybox
        commandBuffer_.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.pipelineLayout, 0, 1, &skyboxRenderPipeline.descriptorSets[0], 0, nullptr);
        commandBuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxRenderPipeline.pipeline);
        skyboxRenderPushConstants.exposure = scene.iblExposure_;
        commandBuffer_.pushConstants(
            skyboxRenderPipeline.pipelineLayout,
            vk::ShaderStageFlagBits::eFragment,
            0,
            sizeof(SkyboxRenderPushConstants),
            &skyboxRenderPushConstants);
        commandBuffer_.bindVertexBuffers(0, 1, &scene.envCube_.vertexBuffers[0]->GetBundle().buffer, vertexOffsets);
        commandBuffer_.bindIndexBuffer(scene.envCube_.indexBuffers[0]->GetBundle().buffer, 0, vk::IndexType::eUint32);
        commandBuffer_.drawIndexed(scene.envCube_.GetIndicesCount(0), scene.envCube_.GetInstanceCount(), 0, 0, 0);
    }

    commandBuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.pipeline);
    commandBuffer_.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshRenderPipeline.pipelineLayout, 0, meshRenderPipeline.descriptorSets.size(), meshRenderPipeline.descriptorSets.data(), 0, nullptr);

    meshRenderPushConsts.iblExposure = scene.iblExposure_;

    // not the actual index
    int meshIndex = 0;
    int materialOffset = 0;
    for (const auto& mesh : scene.meshes_) {
        if (mesh.GetInstanceCount() > 0) {
            meshRenderPushConsts.meshIndex = meshIndex;
            meshRenderPushConsts.lightCount = (int)scene.pointLights_.size();
            meshIndex++;
            for (const auto& part : mesh.GetMeshParts()) {
                commandBuffer_.bindVertexBuffers(0, 1, &mesh.vertexBuffers[part.bufferIndex]->GetBundle().buffer, vertexOffsets);
                commandBuffer_.bindIndexBuffer(mesh.indexBuffers[part.bufferIndex]->GetBundle().buffer, 0, vk::IndexType::eUint32);
                meshRenderPushConsts.materialID = materialOffset + part.materialID;
                commandBuffer_.pushConstants(
                    meshRenderPipeline.pipelineLayout,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(MeshRenderPushConstants),
                    &meshRenderPushConsts);
                commandBuffer_.drawIndexed(mesh.GetIndicesCount(part.bufferIndex), mesh.GetInstanceCount(), 0, 0, 0);
            }
        }
        materialOffset += (int)mesh.GetMaterialCount();
    }

    // physics debug
    commandBuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.pipeline);
    commandBuffer_.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lineRenderPipeline.pipelineLayout, 0, lineRenderPipeline.descriptorSets.size(), lineRenderPipeline.descriptorSets.data(), 0, nullptr);
    commandBuffer_.bindVertexBuffers(0, 1, &scene.physics_.debugDrawer_.m_vertexBuffer->GetBundle().buffer, vertexOffsets);
    commandBuffer_.bindIndexBuffer(scene.physics_.debugDrawer_.m_indexBuffer->GetBundle().buffer, 0, vk::IndexType::eUint32);
    commandBuffer_.drawIndexed(scene.physics_.debugDrawer_.m_lineIndices.size(), 1, 0, 0, 0);

    commandBuffer_.endRenderPass();

    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        viewportImage.GetBundle().image,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    vkn::Command::SetImageMemoryBarrier(commandBuffer_,
                                        colorID.GetBundle().image,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eColorAttachmentRead,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    vkn::Command::Submit(&commandBuffer_, 1);
}

Viewport::~Viewport()
{
    vkn::Device::GetBundle().device.destroyFramebuffer(framebuffer);
    vkn::Device::GetBundle().device.destroyCommandPool(commandPool_);
    vkn::Device::GetBundle().device.destroySampler(vkn::Image::repeatSampler);
    vkn::Device::GetBundle().device.destroySampler(vkn::Image::clampSampler);
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
