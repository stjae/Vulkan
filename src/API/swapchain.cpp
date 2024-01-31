#include "swapchain.h"

Swapchain::Swapchain()
{
    CreateSwapchain();
    CreateRenderPass();
    CreateFrameBuffer();
    PrepareFrames();

    colorPicked_.CreateImage(vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible, { 1, 1, 1 }, vk::ImageTiling::eLinear);
}

void Swapchain::CreateSwapchain()
{
    QuerySwapchainSupport();
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseExtent();

    uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo({}, Instance::GetBundle().surface, imageCount, swapchainBundle_.surfaceFormat.format, swapchainBundle_.surfaceFormat.colorSpace, swapchainBundle_.swapchainImageExtent, 1, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);

    uint32_t indices[] = { Device::GetBundle().graphicsFamilyIndex.value(), Device::GetBundle().presentFamilyIndex.value() };
    if (Device::GetBundle().graphicsFamilyIndex.value() != Device::GetBundle().presentFamilyIndex.value()) {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = indices;
    } else {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchainCreateInfo.presentMode = swapchainBundle_.presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = nullptr;

    swapchainBundle_.swapchain = Device::GetBundle().device.createSwapchainKHR(swapchainCreateInfo);
    Log(debug, fmt::terminal_color::bright_green, "swapchain created");

    // GetBundle swapchain image handle
    std::vector<vk::Image> swapchainImages = Device::GetBundle().device.getSwapchainImagesKHR(swapchainBundle_.swapchain);
    swapchainBundle_.frameImageCount = swapchainImages.size();
    frames.resize(swapchainBundle_.frameImageCount);

    for (size_t i = 0; i < swapchainImages.size(); ++i) {

        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        imageViewCreateInfo.components = vk::ComponentSwizzle::eIdentity;

        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        imageViewCreateInfo.subresourceRange = range;
        imageViewCreateInfo.format = swapchainBundle_.surfaceFormat.format;

        frames[i].swapchainImage = swapchainImages[i];
        frames[i].swapchainImageView = Device::GetBundle().device.createImageView(imageViewCreateInfo);
    }
}

void Swapchain::QuerySwapchainSupport()
{
    capabilities = Device::GetBundle().physicalDevice.getSurfaceCapabilitiesKHR(Instance::GetBundle().surface);
    supportedFormats_ = Device::GetBundle().physicalDevice.getSurfaceFormatsKHR(Instance::GetBundle().surface);
    supportedPresentModes_ = Device::GetBundle().physicalDevice.getSurfacePresentModesKHR(Instance::GetBundle().surface);

    Log(debug, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debug, fmt::terminal_color::white, "current surface extent width: {}", capabilities.currentExtent.width);
    Log(debug, fmt::terminal_color::white, "current surface extent height: {}", capabilities.currentExtent.height);

    for (auto& mode : supportedPresentModes_) {
        Log(debug, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

void Swapchain::ChooseSurfaceFormat()
{
    Log(debug, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : supportedFormats_) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));

            swapchainBundle_.surfaceFormat = format;
            return;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(supportedFormats_[0].format));
    Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(supportedFormats_[0].colorSpace));

    swapchainBundle_.surfaceFormat = supportedFormats_[0].format;
}

void Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    for (auto& presentMode : supportedPresentModes_) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            mode = vk::PresentModeKHR::eMailbox;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));

    swapchainBundle_.presentMode = mode;
}

void Swapchain::ChooseExtent()
{
    // extent is set
    if (capabilities.currentExtent.width != UINT32_MAX) {

        Log(debug, fmt::terminal_color::white, "no change in extent size");

        swapchainBundle_.swapchainImageExtent = capabilities.currentExtent;
        return;

    } else {
        // extent is not set
        int width, height;
        glfwGetFramebufferSize(*Window::GetWindow(), &width, &height);

        vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        if (extent.width > capabilities.maxImageExtent.width) {

            extent.width = capabilities.maxImageExtent.width;
            Log(debug, fmt::terminal_color::yellow, "extent width is clamped");
        }
        if (extent.height > capabilities.maxImageExtent.height) {

            extent.height = capabilities.maxImageExtent.height;
            Log(debug, fmt::terminal_color::yellow, "extent height is clamped");
        }

        swapchainBundle_.swapchainImageExtent = extent;
        return;
    }
}

void Swapchain::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachmentDesc;
    colorAttachmentDesc.format = vk::Format::eB8G8R8A8Unorm;
    colorAttachmentDesc.samples = vk::SampleCountFlagBits::e1;
    colorAttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    std::array<vk::AttachmentDescription, 1> attachments = { colorAttachmentDesc };
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    renderPass_ = Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void Swapchain::CreateFrameBuffer()
{
    for (int i = 0; i < frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            frames[i].swapchainImageView,
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainBundle_.swapchainImageExtent.width;
        framebufferInfo.height = swapchainBundle_.swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        frames[i].framebuffer = Device::GetBundle().device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}

void Swapchain::PrepareFrames()
{
    for (auto& frame : frames) {
        frame.inFlight = MakeFence();
        frame.imageAvailable = MakeSemaphore();
        frame.renderFinished = MakeSemaphore();

        Command::CreateCommandPool(frame.commandPool);
        Command::AllocateCommandBuffer(frame.commandPool, frame.commandBuffer);
        Command::AllocateCommandBuffer(frame.commandPool, frame.renderPassCommandBuffer);
    }
}

void Swapchain::PickColor(size_t frameIndex)
{
    auto& frame = frames[frameIndex];

    Command::Begin(frame.commandBuffer);
    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.swapchainImage,
                                   vk::ImageLayout::ePresentSrcKHR,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   vk::AccessFlagBits::eMemoryRead,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::PipelineStageFlagBits::eTransfer,
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
    glfwGetCursorPos(*Window::GetWindow(), &mouseX, &mouseY);
    glfwGetWindowContentScale(*Window::GetWindow(), &scaleX, &scaleY);
    mouseX *= scaleX;
    mouseY *= scaleY;
    region.srcOffset.x = std::max(0, (int)mouseX);
    region.srcOffset.y = std::max(0, (int)mouseY);
    //    std::cout << (int)mouseX << ' ' << (int)mouseY << '\n';
    region.extent.width = 1;
    region.extent.height = 1;
    region.extent.depth = 1;

    Command::Begin(frame.commandBuffer);
    frame.commandBuffer.copyImage(frame.swapchainImage, vk::ImageLayout::eTransferSrcOptimal, colorPicked_.GetBundle().image, vk::ImageLayout::eTransferDstOptimal, region);

    Command::SetImageMemoryBarrier(frame.commandBuffer,
                                   frame.swapchainImage,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   vk::ImageLayout::ePresentSrcKHR,
                                   vk::AccessFlagBits::eTransferRead,
                                   vk::AccessFlagBits::eMemoryRead,
                                   vk::PipelineStageFlagBits::eTransfer,
                                   vk::PipelineStageFlagBits::eTransfer);
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

    const uint8_t* data;
    Device::GetBundle().device.mapMemory(colorPicked_.memory.Get(), 0, vk::WholeSize, {}, (void**)&data);
    //    data += subResourceLayout.offset;
    std::cout << (int)data[0] << ' ' << (int)data[1] << ' ' << (int)data[2] << '\n';

    //    std::ofstream file("screenShot.ppm", std::ios::out | std::ios::binary);

    // ppm header
    //    file << "P6\n"
    //         << Swapchain::GetBundle().swapchainImageExtent.width << "\n"
    //         << Swapchain::GetBundle().swapchainImageExtent.height << "\n"
    //         << 255 << "\n";
    //
    //    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    //    bool colorSwizzle = false;
    //    // Check if source is BGR
    //    // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
    //    std::vector<vk::Format> formatsBGR = { vk::Format::eB8G8R8A8Srgb, vk::Format::eB8G8R8A8Unorm, vk::Format::eB8G8R8A8Snorm };
    //    colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), Swapchain::GetBundle().surfaceFormat.format) != formatsBGR.end());
    //
    //    // ppm binary pixel data
    //    for (uint32_t y = 0; y < Swapchain::GetBundle().swapchainImageExtent.height; y++) {
    //        unsigned int* row = (unsigned int*)data;
    //        for (uint32_t x = 0; x < Swapchain::GetBundle().swapchainImageExtent.width; x++) {
    //            if (colorSwizzle) {
    //                file.write((char*)row + 2, 1);
    //                file.write((char*)row + 1, 1);
    //                file.write((char*)row, 1);
    //            } else {
    //                file.write((char*)row, 3);
    //            }
    //            row++;
    //        }
    //        data += subResourceLayout.rowPitch;
    //    }
    //    file.close();

    Device::GetBundle().device.unmapMemory(colorPicked_.memory.Get());
}

void Swapchain::Draw(size_t frameIndex, ImDrawData* imDrawData)
{
    auto& frame = frames[frameIndex];

    Command::Begin(frame.renderPassCommandBuffer);
    Command::SetImageMemoryBarrier(frame.renderPassCommandBuffer,
                                   frame.swapchainImage,
                                   vk::ImageLayout::ePresentSrcKHR,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   {}, vk::AccessFlagBits::eColorAttachmentWrite,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = frame.framebuffer;
    vk::Rect2D renderArea(0, 0);
    renderArea.extent = swapchainBundle_.swapchainImageExtent;
    renderPassInfo.renderArea = renderArea;
    vk::ClearValue clearValue;
    clearValue.color = { std::array<float, 4>{ 0.1f, 0.1f, 0.1f, 1.0f } };
    vk::ClearValue depthClear;
    depthClear.depthStencil.depth = 1.0f;
    renderPassInfo.clearValueCount = 2;
    vk::ClearValue clearValues[] = { clearValue, depthClear };
    renderPassInfo.pClearValues = &clearValues[0];

    frame.renderPassCommandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(swapchainBundle_.swapchainImageExtent.height);
    viewport.width = static_cast<float>(swapchainBundle_.swapchainImageExtent.width);
    viewport.height = -1.0f * static_cast<float>(swapchainBundle_.swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = swapchainBundle_.swapchainImageExtent;
    frame.renderPassCommandBuffer.setViewport(0, viewport);
    frame.renderPassCommandBuffer.setScissor(0, scissor);

    ImGui_ImplVulkan_RenderDrawData(imDrawData, frame.renderPassCommandBuffer);

    frame.renderPassCommandBuffer.endRenderPass();

    Command::SetImageMemoryBarrier(frame.renderPassCommandBuffer,
                                   frame.swapchainImage,
                                   vk::ImageLayout::eColorAttachmentOptimal,
                                   vk::ImageLayout::ePresentSrcKHR,
                                   vk::AccessFlagBits::eColorAttachmentWrite, {},
                                   vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                   vk::PipelineStageFlagBits::eBottomOfPipe);
    frame.renderPassCommandBuffer.end();
}

void Swapchain::Submit(size_t frameIndex)
{
    auto& frame = frames[frameIndex];

    vk::SubmitInfo submitInfo;
    vk::Semaphore waitSemaphores[] = { frame.imageAvailable };
    vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.renderPassCommandBuffer;
    vk::Semaphore signalSemaphores[] = { frame.renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    Device::GetBundle().graphicsQueue.submit(submitInfo, frame.inFlight);
}

void Swapchain::Present(size_t frameIndex, const vk::ResultValue<unsigned int>& waitFrameImage)
{
    auto& frame = frames[frameIndex];

    vk::Semaphore signalSemaphores[] = { frame.renderFinished };
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { Swapchain::GetBundle().swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &waitFrameImage.value;

    Device::GetBundle().presentQueue.presentKHR(presentInfo);
}

void Swapchain::Destroy()
{
    for (auto& frame : frames) {
        Device::GetBundle().device.destroyImageView(frame.swapchainImageView);
        Device::GetBundle().device.destroyFramebuffer(frame.framebuffer);
        Device::GetBundle().device.destroyFence(frame.inFlight);
        Device::GetBundle().device.destroySemaphore(frame.imageAvailable);
        Device::GetBundle().device.destroySemaphore(frame.renderFinished);
    }
    Device::GetBundle().device.destroySwapchainKHR(swapchainBundle_.swapchain);
}

Swapchain::~Swapchain()
{
    Destroy();
    for (auto& layout : descriptorSetLayouts_) {
        Device::GetBundle().device.destroyDescriptorSetLayout(layout);
    }
    for (auto& frame : frames)
        Device::GetBundle().device.destroyCommandPool(frame.commandPool);
    Device::GetBundle().device.destroyRenderPass(renderPass_);
}
