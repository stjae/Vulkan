#include "swapchain.h"

Swapchain::Swapchain()
{
    CreateSwapchain();
    CreateRenderPass();
    CreateFrameBuffer();
    PrepareFrames();
}

void Swapchain::CreateSwapchain()
{
    QuerySwapchainSupport();
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseExtent();

    uint32_t imageCount = std::min(swapchainBundle_.support.capabilities.maxImageCount, swapchainBundle_.support.capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo({}, Instance::GetHandle().surface, imageCount, surfaceFormat_.format, surfaceFormat_.colorSpace, swapchainBundle_.swapchainImageExtent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { Queue::GetGraphicsQueueFamilyIndex(), Queue::GetPresentQueueFamilyIndex() };
    if (Queue::GetGraphicsQueueFamilyIndex() != Queue::GetPresentQueueFamilyIndex()) {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = indices;
    } else {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchainCreateInfo.presentMode = presentMode_;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = nullptr;

    swapchainBundle_.swapchain = Device::GetHandle().device.createSwapchainKHR(swapchainCreateInfo);
    Log(debug, fmt::terminal_color::bright_green, "swapchain created");

    // Get swapchain image handle
    std::vector<vk::Image> images = Device::GetHandle().device.getSwapchainImagesKHR(swapchainBundle_.swapchain);
    frames_.resize(images.size());
    swapchainBundle_.frameCount = frames_.size();

    for (size_t i = 0; i < images.size(); ++i) {

        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.image = images[i];
        imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        imageViewCreateInfo.components = vk::ComponentSwizzle::eIdentity;

        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        imageViewCreateInfo.subresourceRange = range;
        imageViewCreateInfo.format = surfaceFormat_.format;

        frames_[i].swapchainImage = images[i];
        frames_[i].swapchainImageView = Device::GetHandle().device.createImageView(imageViewCreateInfo);
    }
}

void Swapchain::QuerySwapchainSupport()
{
    swapchainBundle_.support.capabilities = Device::GetHandle().physicalDevice.getSurfaceCapabilitiesKHR(Instance::GetHandle().surface);
    swapchainBundle_.support.formats = Device::GetHandle().physicalDevice.getSurfaceFormatsKHR(Instance::GetHandle().surface);
    swapchainBundle_.support.presentModes = Device::GetHandle().physicalDevice.getSurfacePresentModesKHR(Instance::GetHandle().surface);

    Log(debug, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debug, fmt::terminal_color::white, "current surface extent width: {}", swapchainBundle_.support.capabilities.currentExtent.width);
    Log(debug, fmt::terminal_color::white, "current surface extent height: {}", swapchainBundle_.support.capabilities.currentExtent.height);

    for (auto& mode : swapchainBundle_.support.presentModes) {
        Log(debug, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

void Swapchain::ChooseSurfaceFormat()
{
    Log(debug, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : swapchainBundle_.support.formats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));

            surfaceFormat_ = format;
            return;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(swapchainBundle_.support.formats[0].format));
    Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(swapchainBundle_.support.formats[0].colorSpace));

    surfaceFormat_ = swapchainBundle_.support.formats[0];
}

void Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    for (auto& presentMode : swapchainBundle_.support.presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            mode = vk::PresentModeKHR::eMailbox;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));

    presentMode_ = mode;
}

void Swapchain::ChooseExtent()
{
    auto& capabilities = swapchainBundle_.support.capabilities;

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

    renderPass_ = Device::GetHandle().device.createRenderPass(renderPassInfo);
}

void Swapchain::CreateFrameBuffer()
{
    for (int i = 0; i < frames_.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            frames_[i].swapchainImageView,
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainBundle_.swapchainImageExtent.width;
        framebufferInfo.height = swapchainBundle_.swapchainImageExtent.height;
        framebufferInfo.layers = 1;

        frames_[i].framebuffer = Device::GetHandle().device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}

void Swapchain::PrepareFrames()
{
    for (auto& frame : frames_) {
        frame.inFlight = MakeFence();
        frame.imageAvailable = MakeSemaphore();
        frame.renderFinished = MakeSemaphore();

        frame.command.CreateCommandPool();
        frame.command.AllocateCommandBuffer();
    }
}

void Swapchain::Draw(size_t frameIndex, const std::vector<Mesh>& meshes, uint32_t dynamicOffsetSize, ImDrawData* imDrawData)
{
    auto& frame = frames_[frameIndex];
    auto& commandBuffer = frame.command.commandBuffers_.back();

    vk::CommandBufferBeginInfo beginInfo;
    commandBuffer.begin(beginInfo);
    {
        vk::ImageMemoryBarrier barrier;

        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barrier.srcQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.dstQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.image = frame.swapchainImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                      vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::DependencyFlagBits::eByRegion, 0, nullptr, 0,
                                      nullptr, 1, &barrier);
    }
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

    commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

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

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    ImGui_ImplVulkan_RenderDrawData(imDrawData, commandBuffer);

    commandBuffer.endRenderPass();
    {
        vk::ImageMemoryBarrier barrier;

        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.dstQueueFamilyIndex = Queue::GetGraphicsQueueFamilyIndex();
        barrier.image = frames_[frameIndex].swapchainImage;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                      vk::PipelineStageFlagBits::eBottomOfPipe,
                                      vk::DependencyFlagBits::eByRegion, 0, nullptr, 0,
                                      nullptr, 1, &barrier);
    }
    commandBuffer.end();
}

void Swapchain::DestroySwapchain()
{
    for (auto& frame : frames_) {
        Device::GetHandle().device.destroyImageView(frame.swapchainImageView);
        Device::GetHandle().device.destroyFramebuffer(frame.framebuffer);
        Device::GetHandle().device.destroyFence(frame.inFlight);
        Device::GetHandle().device.destroySemaphore(frame.imageAvailable);
        Device::GetHandle().device.destroySemaphore(frame.renderFinished);
    }

    Device::GetHandle().device.destroySwapchainKHR(swapchainBundle_.swapchain);
}

Swapchain::~Swapchain()
{
    DestroySwapchain();
    for (auto& layout : descriptorSetLayouts_) {
        Device::GetHandle().device.destroyDescriptorSetLayout(layout);
    }
    Device::GetHandle().device.destroyRenderPass(renderPass_);
}
