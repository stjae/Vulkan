#include "swapchain.h"

void Swapchain::CreateSwapchain(GLFWwindow* window)
{
    QuerySwapchainSupportDetails();

    auto& capabilities = swapchainSupportDetails.capabilities;

    auto surfaceFormat = ChooseSurfaceFormat();
    auto presentMode = ChoosePresentMode();
    auto extent = ChooseExtent(window);

    uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR createInfo({}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
    if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;

    swapchainDetails.swapchain = device.createSwapchainKHR(createInfo);
    Log(debug, fmt::terminal_color::bright_green, "swapchain created");

    std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchainDetails.swapchain);
    swapchainDetails.frames.resize(images.size());

    for (size_t i = 0; i < images.size(); ++i) {

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = images[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.components = vk::ComponentSwizzle::eIdentity;

        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        createInfo.subresourceRange = range;
        createInfo.format = surfaceFormat.format;

        swapchainDetails.frames[i].image = images[i];
        swapchainDetails.frames[i].imageView = device.createImageView(createInfo);
    }
    swapchainDetails.imageFormat = surfaceFormat.format;
    swapchainDetails.extent = extent;
}

void Swapchain::QuerySwapchainSupportDetails()
{
    swapchainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    swapchainSupportDetails.formats = physicalDevice.getSurfaceFormatsKHR(surface);
    swapchainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    Log(debug, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debug, fmt::terminal_color::white, "current surface extent width: {}", swapchainSupportDetails.capabilities.currentExtent.width);
    Log(debug, fmt::terminal_color::white, "current surface extent height: {}", swapchainSupportDetails.capabilities.currentExtent.height);

    for (auto& mode : swapchainSupportDetails.presentModes) {
        Log(debug, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat()
{
    Log(debug, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : swapchainSupportDetails.formats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));
            return format;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(swapchainSupportDetails.formats[0].format));
    Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(swapchainSupportDetails.formats[0].colorSpace));
    return swapchainSupportDetails.formats[0];
}

vk::PresentModeKHR Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    for (auto& presentMode : swapchainSupportDetails.presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            mode = vk::PresentModeKHR::eMailbox;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));
    return mode;
}

vk::Extent2D Swapchain::ChooseExtent(GLFWwindow* window)
{
    auto& capabilities = swapchainSupportDetails.capabilities;

    // extent is set
    if (capabilities.currentExtent.width != UINT32_MAX) {

        Log(debug, fmt::terminal_color::white, "no change in extent size");
        return capabilities.currentExtent;

    } else {
        // extent is not set
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        if (extent.width > capabilities.maxImageExtent.width) {

            extent.width = capabilities.maxImageExtent.width;
            Log(debug, fmt::terminal_color::yellow, "extent width is clamped");
        }
        if (extent.height > capabilities.maxImageExtent.height) {

            extent.height = capabilities.maxImageExtent.height;
            Log(debug, fmt::terminal_color::yellow, "extent height is clamped");
        }

        return extent;
    }
}

void Swapchain::CreateFrameBuffer()
{
    auto& frames = swapchainDetails.frames;

    for (int i = 0; i < frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = { frames[i].imageView };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainDetails.extent.width;
        framebufferInfo.height = swapchainDetails.extent.height;
        framebufferInfo.layers = 1;

        frames[i].framebuffer = device.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}

void Swapchain::PrepareFrames()
{
    for (auto& frame : swapchainDetails.frames) {
        frame.inFlight = MakeFence();
        frame.imageAvailable = MakeSemaphore();
        frame.renderFinished = MakeSemaphore();

        frame.CreateResource();
    }
}

void Swapchain::DestroySwapchain()
{
    for (auto& frame : swapchainDetails.frames) {
        device.destroyImageView(frame.imageView);
        device.destroyFramebuffer(frame.framebuffer);
        device.destroyFence(frame.inFlight);
        device.destroySemaphore(frame.imageAvailable);
        device.destroySemaphore(frame.renderFinished);
        device.freeMemory(frame.cameraDataBuffer.memory);
        device.destroyBuffer(frame.cameraDataBuffer.buffer);
    }

    device.destroySwapchainKHR(swapchainDetails.swapchain);
}

Swapchain::~Swapchain()
{
    DestroySwapchain();
}