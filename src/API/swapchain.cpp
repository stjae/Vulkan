#include "swapchain.h"

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

        vk::Extent3D extent = { swapchainBundle_.swapchainImageExtent.width, swapchainBundle_.swapchainImageExtent.height, 1 };
        if (frames_[i].depthImage.GetHandle().image != VK_NULL_HANDLE) {
            frames_[i].depthImage.DestroyImage();
            frames_[i].depthImage.memory.Free();
        }
        if (frames_[i].depthImage.GetHandle().imageView != VK_NULL_HANDLE) {
            frames_[i].depthImage.DestroyImageView();
        }
        frames_[i].depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        frames_[i].depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
    }

    CreateDescriptorSetLayout();
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

void Swapchain::CreateDescriptorSetLayout()
{
    DescriptorSetLayoutData layout0;
    layout0.descriptorSetCount = 1;

    // descriptor set layout #0
    layout0.indices.push_back(0);
    layout0.descriptorTypes.push_back(vk::DescriptorType::eUniformBuffer);
    layout0.descriptorCounts.push_back(1);
    layout0.bindingStages.push_back(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    layout0.bindingFlags.emplace_back();
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(layout0));
    descriptorSetLayoutData_.push_back(layout0);

    DescriptorSetLayoutData layout1;
    layout1.descriptorSetCount = 1;

    // descriptor set layout #1
    layout1.indices.push_back(0);
    layout1.descriptorTypes.push_back(vk::DescriptorType::eUniformBufferDynamic);
    layout1.descriptorCounts.push_back(1);
    layout1.bindingStages.emplace_back(vk::ShaderStageFlagBits::eVertex);
    layout1.bindingFlags.emplace_back();
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(layout1));
    descriptorSetLayoutData_.push_back(layout1);

    DescriptorSetLayoutData layout2;
    layout2.descriptorSetCount = 1;

    // descriptor set layout #2
    layout2.indices.push_back(0);
    layout2.descriptorTypes.push_back(vk::DescriptorType::eCombinedImageSampler);
    layout2.descriptorCounts.push_back(Device::limits.maxDescriptorSetSamplers);
    layout2.bindingStages.emplace_back(vk::ShaderStageFlagBits::eFragment);
    layout2.bindingFlags.push_back(vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
    layout2.layoutCreateFlags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
    descriptorSetLayouts_.push_back(Descriptor::CreateDescriptorSetLayout(layout2));
    descriptorSetLayoutData_.push_back(layout2);

    descriptorPoolCreateFlags_ = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
}

void Swapchain::CreateFrameBuffer(const vk::RenderPass& vkRenderPass)
{
    for (int i = 0; i < frames_.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            frames_[i].swapchainImageView,
            frames_[i].depthImage.GetHandle().imageView
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = vkRenderPass;
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

        frame.descriptor.CreateDescriptorPool(frames_.size(), descriptorSetLayoutData_, descriptorSetLayouts_, descriptorPoolCreateFlags_);
        frame.descriptor.AllocateDescriptorSets(descriptorSetLayouts_);
    }
}

void Swapchain::RecordDrawCommand(GraphicsPipeline& pipeline, int frameIndex, const std::vector<std::shared_ptr<Mesh>>& meshes, uint32_t dynamicOffsetSize, ImDrawData* imDrawData)
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
    renderPassInfo.renderPass = pipeline.GetHandle().renderPass;
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

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetHandle().pipelineLayout, 0, 1, &frames_[frameIndex].descriptor.descriptorSets_[0], 0, nullptr);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetHandle().pipelineLayout, 2, 1, &frames_[frameIndex].descriptor.descriptorSets_[2], 0, nullptr);

    for (int i = 0; i < meshes.size(); i++) {

        vk::Buffer vertexBuffers[] = { meshes[i]->vertexBuffer->GetHandle().buffer };
        vk::DeviceSize offsets[] = { 0 };

        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        commandBuffer.bindIndexBuffer(meshes[i]->indexBuffer->GetHandle().buffer, 0, vk::IndexType::eUint32);

        uint32_t dynamicOffset = i * dynamicOffsetSize;
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.GetHandle().pipelineLayout, 1, 1, &frames_[frameIndex].descriptor.descriptorSets_[1], 1, &dynamicOffset);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetHandle().pipeline);

        commandBuffer.drawIndexed(static_cast<uint32_t>(meshes[i]->GetIndexCount()), 1, 0, 0, 0);
    }

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
        frame.depthImage.memory.Free();
        frame.depthImage.DestroyImage();
        frame.depthImage.DestroyImageView();
    }

    Device::GetHandle().device.destroySwapchainKHR(swapchainBundle_.swapchain);
}

Swapchain::~Swapchain()
{
    DestroySwapchain();
    for (auto& layout : descriptorSetLayouts_) {
        Device::GetHandle().device.destroyDescriptorSetLayout(layout);
    }
}
