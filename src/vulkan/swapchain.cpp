#include "swapchain.h"
#include "instance.h"
#include "device.h"
#include "command.h"
#include "image.h"

vkn::Swapchain::Swapchain()
{
    CreateSwapchain();
    CreateRenderPass();
    CreateFrameBuffer();
    PrepareFrames();
}

void vkn::Swapchain::CreateSwapchain()
{
    QuerySwapchainSupport();
    ChooseSurfaceFormat();
    ChoosePresentMode();
    ChooseExtent();

    uint32_t imageCount = std::min(surfaceCapabilities.maxImageCount, surfaceCapabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo({}, vkn::Instance::GetBundle().surface, imageCount, swapchainBundle_.surfaceFormat.format, swapchainBundle_.surfaceFormat.colorSpace, swapchainBundle_.swapchainImageExtent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { vkn::Device::GetBundle().graphicsFamilyIndex.value(), vkn::Device::GetBundle().presentFamilyIndex.value() };
    if (vkn::Device::GetBundle().graphicsFamilyIndex.value() != vkn::Device::GetBundle().presentFamilyIndex.value()) {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = indices;
    } else {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapchainCreateInfo.presentMode = swapchainBundle_.presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = nullptr;

    swapchainBundle_.swapchain = vkn::Device::GetBundle().device.createSwapchainKHR(swapchainCreateInfo);
    Log(debugMode, fmt::terminal_color::bright_green, "swapchain created");

    // GetBundle swapchain image handle
    std::vector<vk::Image> swapchainImages = vkn::Device::GetBundle().device.getSwapchainImagesKHR(swapchainBundle_.swapchain);
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
        frames[i].swapchainImageView = vkn::Device::GetBundle().device.createImageView(imageViewCreateInfo);
    }
}

void vkn::Swapchain::QuerySwapchainSupport()
{
    surfaceCapabilities = vkn::Device::GetBundle().physicalDevice.getSurfaceCapabilitiesKHR(vkn::Instance::GetBundle().surface);
    supportedFormats_ = vkn::Device::GetBundle().physicalDevice.getSurfaceFormatsKHR(vkn::Instance::GetBundle().surface);
    supportedPresentModes_ = vkn::Device::GetBundle().physicalDevice.getSurfacePresentModesKHR(vkn::Instance::GetBundle().surface);

    Log(debugMode, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debugMode, fmt::terminal_color::white, "current surface extent width: {}", surfaceCapabilities.currentExtent.width);
    Log(debugMode, fmt::terminal_color::white, "current surface extent height: {}", surfaceCapabilities.currentExtent.height);

    for (auto& mode : supportedPresentModes_) {
        Log(debugMode, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

void vkn::Swapchain::ChooseSurfaceFormat()
{
    Log(debugMode, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : supportedFormats_) {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(debugMode, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(debugMode, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));

            swapchainBundle_.surfaceFormat = format;
            return;
        }
    }

    Log(debugMode, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(supportedFormats_[0].format));
    Log(debugMode, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(supportedFormats_[0].colorSpace));

    swapchainBundle_.surfaceFormat = supportedFormats_[0].format;
}

void vkn::Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    for (auto& presentMode : supportedPresentModes_) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            mode = vk::PresentModeKHR::eMailbox;
        }
    }

    Log(debugMode, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));

    swapchainBundle_.presentMode = mode;
}

void vkn::Swapchain::ChooseExtent()
{
    // extent is set
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {

        Log(debugMode, fmt::terminal_color::white, "no change in extent range");

        swapchainBundle_.swapchainImageExtent = surfaceCapabilities.currentExtent;
        return;

    } else {
        // extent is not set
        int width, height;
        glfwGetFramebufferSize(Window::GetWindow(), &width, &height);

        vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        if (extent.width > surfaceCapabilities.maxImageExtent.width) {

            extent.width = surfaceCapabilities.maxImageExtent.width;
            Log(debugMode, fmt::terminal_color::yellow, "extent width is clamped");
        }
        if (extent.height > surfaceCapabilities.maxImageExtent.height) {

            extent.height = surfaceCapabilities.maxImageExtent.height;
            Log(debugMode, fmt::terminal_color::yellow, "extent height is clamped");
        }

        swapchainBundle_.swapchainImageExtent = extent;
        return;
    }
}

void vkn::Swapchain::CreateRenderPass()
{
    vk::AttachmentDescription swapchainAttachment;
    swapchainAttachment.format = vk::Format::eB8G8R8A8Srgb;
    swapchainAttachment.samples = vk::SampleCountFlagBits::e1;
    swapchainAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    swapchainAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    swapchainAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    swapchainAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    swapchainAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
    swapchainAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc;
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &swapchainAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;

    renderPass_ = vkn::Device::GetBundle().device.createRenderPass(renderPassInfo);
}

void vkn::Swapchain::CreateFrameBuffer()
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

        frames[i].framebuffer = vkn::Device::GetBundle().device.createFramebuffer(framebufferInfo);
        Log(debugMode, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}

void vkn::Swapchain::PrepareFrames()
{
    for (auto& frame : frames) {
        frame.inFlight = CreateFence();
        frame.imageAvailable = CreateSemaphore();
        frame.renderFinished = CreateSemaphore();

        vkn::Command::CreateCommandPool(frame.commandPool);
        vkn::Command::AllocateCommandBuffer(frame.commandPool, frame.commandBuffer);
        vkn::Command::AllocateCommandBuffer(frame.commandPool, frame.renderPassCommandBuffer);
    }
}

void vkn::Swapchain::Draw(size_t frameIndex, ImDrawData* imDrawData)
{
    auto& frame = frames[frameIndex];

    vkn::Command::Begin(frame.renderPassCommandBuffer);
    vkn::Command::SetImageMemoryBarrier(frame.renderPassCommandBuffer,
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

    vkn::Command::SetImageMemoryBarrier(frame.renderPassCommandBuffer,
                                        frame.swapchainImage,
                                        vk::ImageLayout::eColorAttachmentOptimal,
                                        vk::ImageLayout::ePresentSrcKHR,
                                        vk::AccessFlagBits::eColorAttachmentWrite, {},
                                        vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                        vk::PipelineStageFlagBits::eBottomOfPipe);
    frame.renderPassCommandBuffer.end();
}

void vkn::Swapchain::Submit(size_t frameIndex)
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

    vkn::Device::GetBundle().graphicsQueue.submit(submitInfo, frame.inFlight);
}

vk::PresentInfoKHR vkn::Swapchain::Present(size_t frameIndex, const vk::ResultValue<unsigned int>& waitFrameImage)
{
    auto& frame = frames[frameIndex];

    vk::Semaphore signalSemaphores[] = { frame.renderFinished };
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vk::SwapchainKHR swapchains[] = { vkn::Swapchain::GetBundle().swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &waitFrameImage.value;
    vkn::Device::GetBundle().presentQueue.presentKHR(presentInfo);

    return presentInfo;
}

void vkn::Swapchain::Destroy()
{
    for (auto& frame : frames) {
        vkn::Device::GetBundle().device.destroyImageView(frame.swapchainImageView);
        vkn::Device::GetBundle().device.destroyFramebuffer(frame.framebuffer);
        vkn::Device::GetBundle().device.destroyFence(frame.inFlight);
        vkn::Device::GetBundle().device.destroySemaphore(frame.imageAvailable);
        vkn::Device::GetBundle().device.destroySemaphore(frame.renderFinished);
    }
    vkn::Device::GetBundle().device.destroySwapchainKHR(swapchainBundle_.swapchain);
}

vkn::Swapchain::~Swapchain()
{
    Destroy();
    for (auto& layout : descriptorSetLayouts_) {
        vkn::Device::GetBundle().device.destroyDescriptorSetLayout(layout);
    }
    for (auto& frame : frames)
        vkn::Device::GetBundle().device.destroyCommandPool(frame.commandPool);
    vkn::Device::GetBundle().device.destroyRenderPass(renderPass_);
    vkn::Device::GetBundle().device.destroySampler(vkn::ImageBundle::globalSampler);
}
