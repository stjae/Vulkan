#include "swapchain.h"

void Swapchain::CreateSwapchain(GLFWwindow* window, const Device& device)
{
    QueryswapchainSupportDetails(device);

    auto& capabilities = supportDetail.capabilities;

    auto surfaceFormat = ChooseSurfaceFormat();
    auto presentMode = ChoosePresentMode();
    auto extent = ChooseExtent(window);

    uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR createInfo({}, vkSurface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { device.queueFamilyIndices.graphicsFamily.value(), device.queueFamilyIndices.presentFamily.value() };
    if (device.queueFamilyIndices.graphicsFamily.value() != device.queueFamilyIndices.presentFamily.value()) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;

    vkSwapchain = device.vkDevice.createSwapchainKHR(createInfo);
    Log(debug, fmt::terminal_color::bright_green, "swapchain created");

    std::vector<vk::Image> images = device.vkDevice.getSwapchainImagesKHR(vkSwapchain);
    detail.frames.resize(images.size(), SwapchainFrame(vkPhysicalDevice, vkDevice));
    detail.imageFormat = surfaceFormat.format;
    detail.extent = extent;

    for (size_t i = 0; i < images.size(); ++i) {

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = images[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.components = vk::ComponentSwizzle::eIdentity;

        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        createInfo.subresourceRange = range;
        createInfo.format = surfaceFormat.format;

        detail.frames[i].swapchainVkImage = images[i];
        detail.frames[i].swapchainVkImageView = device.vkDevice.createImageView(createInfo);

        vk::Extent3D extent = { detail.extent.width, detail.extent.height, 1 };
        detail.frames[i].depthImage.CreateImage(vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        detail.frames[i].depthImage.CreateImageView(vk::Format::eD32Sfloat, vk::ImageAspectFlagBits::eDepth);
    }
}

void Swapchain::QueryswapchainSupportDetails(const Device& device)
{
    supportDetail.capabilities = device.vkPhysicalDevice.getSurfaceCapabilitiesKHR(vkSurface);
    supportDetail.formats = device.vkPhysicalDevice.getSurfaceFormatsKHR(vkSurface);
    supportDetail.presentModes = device.vkPhysicalDevice.getSurfacePresentModesKHR(vkSurface);

    Log(debug, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debug, fmt::terminal_color::white, "current surface extent width: {}", supportDetail.capabilities.currentExtent.width);
    Log(debug, fmt::terminal_color::white, "current surface extent height: {}", supportDetail.capabilities.currentExtent.height);

    for (auto& mode : supportDetail.presentModes) {
        Log(debug, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat()
{
    Log(debug, fmt::terminal_color::black, "setting swapchain details..");

    for (auto& format : supportDetail.formats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(format.format));
            Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(format.colorSpace));
            return format;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set surface pixel format: {}", vk::to_string(supportDetail.formats[0].format));
    Log(debug, fmt::terminal_color::bright_cyan, "set surface color space: {}", vk::to_string(supportDetail.formats[0].colorSpace));
    return supportDetail.formats[0];
}

vk::PresentModeKHR Swapchain::ChoosePresentMode()
{
    vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;

    for (auto& presentMode : supportDetail.presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            mode = vk::PresentModeKHR::eMailbox;
        }
    }

    Log(debug, fmt::terminal_color::bright_cyan, "set swapchain present mode: {}", vk::to_string(mode));
    return mode;
}

vk::Extent2D Swapchain::ChooseExtent(GLFWwindow* window)
{
    auto& capabilities = supportDetail.capabilities;

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
    for (int i = 0; i < detail.frames.size(); ++i) {

        std::vector<vk::ImageView> attachments = {
            detail.frames[i].swapchainVkImageView,
            detail.frames[i].depthImage.imageView
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = vkRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = detail.extent.width;
        framebufferInfo.height = detail.extent.height;
        framebufferInfo.layers = 1;

        detail.frames[i].framebuffer = vkDevice.createFramebuffer(framebufferInfo);
        Log(debug, fmt::terminal_color::bright_green, "created framebuffer for frame {}", i);
    }
}

void Swapchain::PrepareFrames()
{
    for (auto& frame : detail.frames) {
        frame.inFlight = MakeFence(vkDevice);
        frame.imageAvailable = MakeSemaphore(vkDevice);
        frame.renderFinished = MakeSemaphore(vkDevice);

        frame.CreateResource(vkPhysicalDevice, vkDevice);
    }
}

void Swapchain::DestroySwapchain()
{
    for (auto& frame : detail.frames) {
        vkDevice.destroyImageView(frame.swapchainVkImageView);
        vkDevice.destroyFramebuffer(frame.framebuffer);
        vkDevice.destroyFence(frame.inFlight);
        vkDevice.destroySemaphore(frame.imageAvailable);
        vkDevice.destroySemaphore(frame.renderFinished);
    }

    vkDevice.destroySwapchainKHR(vkSwapchain);
}

Swapchain::~Swapchain()
{
    DestroySwapchain();
}