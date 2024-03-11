#include "viewport.h"

Viewport::Viewport()
{
    extent = vk::Extent2D((uint32_t)Swapchain::GetBundle().swapchainImageExtent.width, (uint32_t)Swapchain::GetBundle().swapchainImageExtent.height);

    CreateViewportRenderPass();
    pipelineState_.meshRender.CreateMeshRenderDescriptorSetLayout();
    pipelineState_.meshRender.CreateMeshRenderPipeline(viewportRenderPass_, "shaders/base.vert.spv", "shaders/base.frag.spv");

    Command::CreateCommandPool(commandPool_);
    Command::AllocateCommandBuffer(commandPool_, commandBuffer_);

    CreateViewportImages();

    colorPicked_.CreateImage(vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, { 1, 1, 1 }, vk::ImageTiling::eLinear);

    PrepareShadowCubeMap();
    CreateShadowMapRenderPass();
    // pipelineState_.shadowMap.CreateShadowMapDescriptorSetLayout();
    // pipelineState_.shadowMap.CreateShadowMapPipeline(shadowMapPass_.renderPass,"");
}

void Viewport::CreateViewportImages()
{
    viewportImage.CreateImage(vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, { extent.width, extent.height, 1 }, vk::ImageTiling::eOptimal);
    viewportImage.CreateImageView(vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor);
    if (viewportImage.GetBundle().sampler == VK_NULL_HANDLE)
        viewportImage.CreateSampler();

    colorID.CreateImage(vk::Format::eR32G32Sint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, { extent.width, extent.height, 1 }, vk::ImageTiling::eOptimal);
    colorID.CreateImageView(vk::Format::eR32G32Sint, vk::ImageAspectFlagBits::eColor);
    if (colorID.GetBundle().sampler == VK_NULL_HANDLE)
        colorID.CreateSampler();

    depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, { extent.width, extent.height, 1 }, vk::ImageTiling::eOptimal);
    depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);

    Command::Begin(commandBuffer_);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   viewportImage,
                                   {},
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   {},
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorID,
                                   {},
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   {},
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    CreateViewportFrameBuffer();
}

void Viewport::DestroyViewportImages()
{
    Device::GetBundle().device.destroyFramebuffer(framebuffer);

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

void Viewport::CreateViewportRenderPass()
{
    std::array<vk::AttachmentDescription, 3> viewportAttachments;

    viewportAttachments[0].format = vk::Format::eB8G8R8A8Srgb;
    viewportAttachments[0].samples = vk::SampleCountFlagBits::e1;
    viewportAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    viewportAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    viewportAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    viewportAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    viewportAttachments[0].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    viewportAttachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // ID
    viewportAttachments[1].format = vk::Format::eR32G32Sint;
    viewportAttachments[1].samples = vk::SampleCountFlagBits::e1;
    viewportAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    viewportAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    viewportAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    viewportAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    viewportAttachments[1].initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    viewportAttachments[1].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    // Depth
    viewportAttachments[2].format = vk::Format::eD32Sfloat;
    viewportAttachments[2].samples = vk::SampleCountFlagBits::e1;
    viewportAttachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    viewportAttachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    viewportAttachments[2].stencilLoadOp = vk::AttachmentLoadOp::eClear;
    viewportAttachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    viewportAttachments[2].initialLayout = vk::ImageLayout::eUndefined;
    viewportAttachments[2].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference idAttachmentRef;
    idAttachmentRef.attachment = 1;
    idAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    std::array<vk::AttachmentReference, 2> colorAttachmentRefs{ colorAttachmentRef, idAttachmentRef };

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = colorAttachmentRefs.size();
    subpassDesc.pColorAttachments = colorAttachmentRefs.data();
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = (uint32_t)(viewportAttachments.size());
    renderPassInfo.pAttachments = viewportAttachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    viewportRenderPass_ = Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void Viewport::CreateViewportFrameBuffer()
{
    std::vector<vk::ImageView> attachments = {
        viewportImage.GetBundle().imageView,
        colorID.GetBundle().imageView,
        depthImage.GetBundle().imageView,
    };

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.renderPass = viewportRenderPass_;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    framebuffer = Device::GetBundle().device.createFramebuffer(framebufferInfo);
}

const int32_t* Viewport::PickColor(size_t frameIndex, double mouseX, double mouseY)
{
    Command::Begin(commandBuffer_);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorID,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   vk::PipelineStageFlagBits::eTransfer);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorPicked_.GetBundle().image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eTransfer);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

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

    Command::Begin(commandBuffer_);
    commandBuffer_.copyImage(colorID.GetBundle().image, vk::ImageLayout::eTransferSrcOptimal, colorPicked_.GetBundle().image, vk::ImageLayout::eTransferDstOptimal, region);

    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorID,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorPicked_.GetBundle().image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eGeneral,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::AccessFlagBits::eMemoryRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eTransfer);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    vk::ImageSubresource subResource{ vk::ImageAspectFlagBits::eColor, 0, 0 };
    vk::SubresourceLayout subResourceLayout;
    Device::GetBundle().device.getImageSubresourceLayout(colorPicked_.GetBundle().image, &subResource, &subResourceLayout);

    const int32_t* data;
    Device::GetBundle().device.mapMemory(colorPicked_.memory.GetMemory(), 0, vk::WholeSize, {}, (void**)&data);
    std::cout << "mesh id: " << data[0] << " instance id: " << data[1] << '\n';
    Device::GetBundle().device.unmapMemory(colorPicked_.memory.GetMemory());

    return data;
}

void Viewport::Draw(size_t frameIndex, Scene& scene)
{
    Command::Begin(commandBuffer_);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   viewportImage,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorID,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = viewportRenderPass_;
    renderPassInfo.framebuffer = framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = extent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue colorClearValue;
    vk::ClearValue idClearValue;
    colorClearValue.color = { std::array<float, 4>{ 0.1f, 0.1f, 0.1f, 1.0f } };
    idClearValue.color = { std::array<int32_t, 4>{ -1, -1, -1, -1 } };
    vk::ClearValue depthClearValue;
    depthClearValue.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 3;
    vk::ClearValue clearValues[] = { colorClearValue, idClearValue, depthClearValue };
    renderPassInfo.pClearValues = clearValues;

    commandBuffer_.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.height);
    viewport.width = static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.width);
    viewport.height = -1.0f * static_cast<float>(Swapchain::GetBundle().swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = Swapchain::GetBundle().swapchainImageExtent;

    commandBuffer_.setViewport(0, viewport);
    commandBuffer_.setScissor(0, scissor);

    commandBuffer_.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipelineLayout, 1, 1, &pipelineState_.meshRender.descriptorSets[1], 0, nullptr);

    commandBuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipeline);
    vk::DeviceSize vertexOffsets[]{ 0 };

    uint32_t dynamicOffset = 0;
    for (auto& mesh : scene.meshes) {
        if (mesh.GetInstanceCount() < 1)
            continue;
        commandBuffer_.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipelineLayout, 0, 1, &pipelineState_.meshRender.descriptorSets[0], 1, &dynamicOffset);
        commandBuffer_.bindVertexBuffers(0, 1, &mesh.vertexBuffer->GetBundle().buffer, vertexOffsets);
        commandBuffer_.bindIndexBuffer(mesh.indexBuffer->GetBundle().buffer, 0, vk::IndexType::eUint32);
        commandBuffer_.drawIndexed(mesh.GetIndexCount(), mesh.GetInstanceCount(), 0, 0, 0);

        dynamicOffset += mesh.GetInstanceCount() * sizeof(InstanceData);
    }

    commandBuffer_.endRenderPass();

    Command::SetImageMemoryBarrier(commandBuffer_,
                                   viewportImage,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   colorID,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);
}

void Viewport::PrepareShadowCubeMap()
{
    shadowMapSize_ = 1024;
    shadowMapImageFormat_ = vk::Format::eR32Sfloat;

    vk::ImageCreateInfo shadowCubeMapImageCI(vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, shadowMapImageFormat_, { shadowMapSize_, shadowMapSize_, 1 }, 1, 6, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive);
    shadowCubeMap_.CreateImage(shadowCubeMapImageCI, vk::MemoryPropertyFlagBits::eDeviceLocal);
    Command::Begin(commandBuffer_);
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
    Command::SetImageMemoryBarrier(commandBuffer_,
                                   shadowCubeMap_,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   vk::PipelineStageFlagBits::eAllCommands,
                                   subresourceRange);
    commandBuffer_.end();
    Command::Submit(&commandBuffer_, 1);

    shadowCubeMap_.CreateSampler();

    vk::ImageViewCreateInfo viewCI({}, shadowCubeMap_.GetBundle().image, vk::ImageViewType::eCube, shadowMapImageFormat_, { vk::ComponentSwizzle::eR }, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6 });
    shadowCubeMap_.CreateImageView(viewCI);

    viewCI.viewType = vk::ImageViewType::e2D;
    viewCI.subresourceRange.layerCount = 1;

    for (uint32_t i = 0; i < 6; i++) {
        viewCI.subresourceRange.baseArrayLayer = i;
        Device::GetBundle().device.createImageView(&viewCI, nullptr, &shadowCubeMapFaceImageViews_[i]);
    }
}

void Viewport::CreateShadowMapRenderPass()
{
    std::array<vk::AttachmentDescription, 2> shadowMapAttachments;

    shadowMapAttachments[0].format = shadowMapImageFormat_;
    shadowMapAttachments[0].samples = vk::SampleCountFlagBits::e1;
    shadowMapAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    shadowMapAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    shadowMapAttachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    shadowMapAttachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    shadowMapAttachments[0].initialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    shadowMapAttachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Depth
    shadowMapAttachments[1].format = vk::Format::eD32Sfloat;
    shadowMapAttachments[1].samples = vk::SampleCountFlagBits::e1;
    shadowMapAttachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    shadowMapAttachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    shadowMapAttachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    shadowMapAttachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    shadowMapAttachments[1].initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    shadowMapAttachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDesc{};
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassCI{};
    renderPassCI.attachmentCount = shadowMapAttachments.size();
    renderPassCI.pAttachments = shadowMapAttachments.data();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpassDesc;

    Device::GetBundle().device.createRenderPass(&renderPassCI, nullptr, &shadowMapPass_.renderPass);
}

Viewport::~Viewport()
{
    Device::GetBundle().device.destroyRenderPass(viewportRenderPass_);
    Device::GetBundle().device.destroyFramebuffer(framebuffer);
    Device::GetBundle().device.destroyCommandPool(commandPool_);

    Device::GetBundle().device.destroyRenderPass(shadowMapPass_.renderPass);
    for (auto& view : shadowCubeMapFaceImageViews_) {
        Device::GetBundle().device.destroyImageView(view);
    }
}
