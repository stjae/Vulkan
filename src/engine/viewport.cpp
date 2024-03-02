#include "viewport.h"

Viewport::Viewport()
{
    frames.resize(Swapchain::GetBundle().frameImageCount);
    extent = vk::Extent2D((uint32_t)Swapchain::GetBundle().swapchainImageExtent.width, (uint32_t)Swapchain::GetBundle().swapchainImageExtent.height);

    CreateRenderPass();
    pipelineState_.meshRender.CreateMeshRenderDescriptorSetLayout();
    pipelineState_.meshRender.CreateGraphicsPipeline(viewportRenderPass_, "shaders/base.vert.spv", "shaders/base.frag.spv", vk::PrimitiveTopology::eTriangleList);

    for (auto& frame : frames) {

        Command::CreateCommandPool(frame.commandPool);
        Command::AllocateCommandBuffer(frame.commandPool, frame.commandBuffer);
    }

    CreateViewportImages();

    colorPicked_.CreateImage(vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, { 1, 1, 1 }, vk::ImageTiling::eLinear);
}

void Viewport::CreateViewportImages()
{
    for (auto& frame : frames) {
        frame.viewportImage.CreateImage(vk::Format::eB8G8R8A8Srgb, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, { extent.width, extent.height, 1 }, vk::ImageTiling::eOptimal);
        frame.viewportImage.CreateImageView(vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor);
        if (frame.viewportImage.GetBundle().sampler == VK_NULL_HANDLE)
            frame.viewportImage.CreateSampler();

        frame.colorID.CreateImage(vk::Format::eR32Sint, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, { extent.width, extent.height, 1 }, vk::ImageTiling::eOptimal);
        frame.colorID.CreateImageView(vk::Format::eR32Sint, vk::ImageAspectFlagBits::eColor);
        if (frame.colorID.GetBundle().sampler == VK_NULL_HANDLE)
            frame.colorID.CreateSampler();

        frame.depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, { extent.width, extent.height, 1 }, vk::ImageTiling::eOptimal);
        frame.depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);

        Command::Begin(frame.commandBuffer);
        Command::SetImageMemoryBarrier(frame.commandBuffer,
                                       frame.viewportImage,
                                       {},
                                       vk::ImageLayout::eShaderReadOnlyOptimal,
                                       {},
                                       vk::AccessFlagBits::eShaderRead,
                                       vk::PipelineStageFlagBits::eTopOfPipe,
                                       vk::PipelineStageFlagBits::eFragmentShader);
        Command::SetImageMemoryBarrier(frame.commandBuffer,
                                       frame.colorID,
                                       {},
                                       vk::ImageLayout::eShaderReadOnlyOptimal,
                                       {},
                                       vk::AccessFlagBits::eShaderRead,
                                       vk::PipelineStageFlagBits::eTopOfPipe,
                                       vk::PipelineStageFlagBits::eFragmentShader);
        frame.commandBuffer.end();
        Command::Submit(&frame.commandBuffer, 1);
    }

    CreateFrameBuffer();
}

void Viewport::DestroyViewportImages()
{
    for (auto& frame : frames) {
        Device::GetBundle().device.destroyFramebuffer(frame.framebuffer);

        frame.viewportImage.DestroyImage();
        frame.viewportImage.DestroyImageView();
        frame.viewportImage.memory.Free();

        frame.colorID.DestroyImage();
        frame.colorID.DestroyImageView();
        frame.colorID.memory.Free();

        frame.depthImage.DestroyImage();
        frame.depthImage.DestroyImageView();
        frame.depthImage.memory.Free();
    }
}

void Viewport::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = vk::Format::eB8G8R8A8Srgb;
    colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription idAttachmentDesc;
    idAttachmentDesc.format = vk::Format::eR32Sint;
    idAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    idAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    idAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    idAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    idAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    idAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    idAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentDescription depthAttachmentDesc;
    depthAttachmentDesc.format = vk::Format::eD32Sfloat;
    depthAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    depthAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eClear;
    depthAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachmentDesc.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

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

    std::array<vk::AttachmentDescription, 3> attachments = { colorAttachmentDesc, idAttachmentDesc, depthAttachmentDesc };
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    viewportRenderPass_ = Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void Viewport::CreateFrameBuffer()
{
    for (auto& frame : frames) {

        std::vector<vk::ImageView> attachments = {
            frame.viewportImage.GetBundle().imageView,
            frame.colorID.GetBundle().imageView,
            frame.depthImage.GetBundle().imageView,
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = viewportRenderPass_;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        frame.framebuffer = Device::GetBundle().device.createFramebuffer(framebufferInfo);
    }
}

int32_t Viewport::PickColor(size_t frameIndex)
{
    auto& frame = frames[frameIndex];

    Command::Begin(frame.commandBuffer);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.colorID,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   vk::PipelineStageFlagBits::eTransfer);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   colorPicked_.GetBundle().image,
                                   vk::ImageLayout::eUndefined,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   {},
                                   vk::AccessFlagBits::eTransferWrite,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eTransfer);
    frame.commandBuffer.end();
    Command::Submit(&frame.commandBuffer, 1);

    vk::ImageCopy region;
    region.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.srcSubresource.layerCount = 1;
    region.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.dstSubresource.layerCount = 1;
    float scaleX, scaleY;
    double mouseX, mouseY;
    glfwGetCursorPos(Window::GetWindow(), &mouseX, &mouseY);
    glfwGetWindowContentScale(Window::GetWindow(), &scaleX, &scaleY);
    int32_t offsetX = 0, offsetY = 0;
    if (mouseX > panelPos.x)
        offsetX = (int32_t)((mouseX - panelPos.x));
    if (mouseY > panelPos.y)
        offsetY = (int32_t)((mouseY - panelPos.y));
    region.srcOffset.x = offsetX;
    region.srcOffset.y = offsetY;
    region.extent.width = 1;
    region.extent.height = 1;
    region.extent.depth = 1;

    Command::Begin(frame.commandBuffer);
    frame.commandBuffer.copyImage(frame.colorID.GetBundle().image, vk::ImageLayout::eTransferSrcOptimal, colorPicked_.GetBundle().image, vk::ImageLayout::eTransferDstOptimal, region);

    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.colorID,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   colorPicked_.GetBundle().image,
                                   vk::ImageLayout::eTransferDstOptimal,
                                   vk::ImageLayout::eGeneral,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::AccessFlagBits::eMemoryRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eTransfer);
    frame.commandBuffer.end();
    Command::Submit(&frame.commandBuffer, 1);

    vk::ImageSubresource subResource{ vk::ImageAspectFlagBits::eColor, 0, 0 };
    vk::SubresourceLayout subResourceLayout;
    Device::GetBundle().device.getImageSubresourceLayout(colorPicked_.GetBundle().image, &subResource, &subResourceLayout);

    const int32_t* data;
    Device::GetBundle().device.mapMemory(colorPicked_.memory.GetMemory(), 0, vk::WholeSize, {}, (void**)&data);
    int32_t meshID = data[0];
    std::cout << meshID << '\n';
    Device::GetBundle().device.unmapMemory(colorPicked_.memory.GetMemory());

    return meshID;
}

void Viewport::Draw(size_t frameIndex, Scene& scene)
{
    auto& frame = frames[frameIndex];

    Command::Begin(frame.commandBuffer);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.viewportImage,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.colorID,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::PipelineStageFlagBits::eFragmentShader,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = viewportRenderPass_;
    renderPassInfo.framebuffer = frame.framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = extent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue colorClearValue;
    vk::ClearValue idClearValue;
    colorClearValue.color = { std::array<float, 4>{ 0.25f, 0.25f, 0.25f, 1.0f } };
    idClearValue.color = { std::array<int32_t, 4>{ -1, -1, -1, -1 } };
    vk::ClearValue depthClearValue;
    depthClearValue.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 3;
    vk::ClearValue clearValues[] = { colorClearValue, idClearValue, depthClearValue };
    renderPassInfo.pClearValues = clearValues;

    frame.commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

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

    frame.commandBuffer.setViewport(0, viewport);
    frame.commandBuffer.setScissor(0, scissor);

    frame.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipelineLayout, 0, 1, &pipelineState_.meshRender.descriptorSets[0], 0, nullptr);
    frame.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipelineLayout, 2, 1, &pipelineState_.meshRender.descriptorSets[2], 0, nullptr);

    frame.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipeline);

    for (int i = 0; i < scene.meshes.size(); i++) {

        vk::DeviceSize vertexOffsets[]{ 0 };

        frame.commandBuffer.bindVertexBuffers(0, 1, &scene.meshes[i].vertexBuffer->GetBundle().buffer, vertexOffsets);
        frame.commandBuffer.bindIndexBuffer(scene.meshes[i].indexBuffer->GetBundle().buffer, 0, vk::IndexType::eUint32);

        uint32_t dynamicOffset = i * scene.GetDynamicMeshUniformRange();

        frame.commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineState_.meshRender.GetBundle().pipelineLayout, 1, 1, &pipelineState_.meshRender.descriptorSets[1], 1, &dynamicOffset);

        frame.commandBuffer.drawIndexed(scene.meshes[i].GetIndexCount(), 1, 0, 0, 0);
    }

    frame.commandBuffer.endRenderPass();

    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.viewportImage,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.colorID,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::eShaderReadOnlyOptimal,
                                   vk::AccessFlagBits::eColorAttachmentRead,
                                   vk::AccessFlagBits::eShaderRead,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                   vk::PipelineStageFlagBits::eFragmentShader);
    frame.commandBuffer.end();
    Command::Submit(&frame.commandBuffer, 1);
}

Viewport::~Viewport()
{
    Device::GetBundle().device.destroyRenderPass(viewportRenderPass_);
    for (auto& frame : frames) {
        Device::GetBundle().device.destroyFramebuffer(frame.framebuffer);
        Device::GetBundle().device.destroyCommandPool(frame.commandPool);
    }
}
