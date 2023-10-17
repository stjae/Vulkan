#include "swapchain.h"

void Swapchain::QuerySwapchainSupportDetails()
{
    swapchainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    swapchainSupportDetails.formats = physicalDevice.getSurfaceFormatsKHR(surface);
    swapchainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    auto& capabilities = swapchainSupportDetails.capabilities;
    auto& formats = swapchainSupportDetails.formats;
    auto& presentModes = swapchainSupportDetails.presentModes;

    Log(debug, fmt::terminal_color::black, "printing queries for surface supports..");

    Log(debug, fmt::terminal_color::white, "current surface extent width: {}", capabilities.currentExtent.width);
    Log(debug, fmt::terminal_color::white, "current surface extent height: {}", capabilities.currentExtent.height);

    for (auto& mode : presentModes) {
        Log(debug, fmt::terminal_color::white, "supported present mode: {}", vk::to_string(mode));
    }
}

void Swapchain::CreateSwapchain(GLFWwindow* window)
{
    QuerySwapchainSupportDetails();

    auto& capabilities = swapchainSupportDetails.capabilities;

    auto surfaceFormat = ChooseSurfaceFormat();
    auto presentMode = ChoosePresentMode();
    auto extent = ChooseExtent(window);

    uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setSurface(surface);
    createInfo.setMinImageCount(imageCount);
    createInfo.setImageFormat(surfaceFormat.format);
    createInfo.setImageColorSpace(surfaceFormat.colorSpace);
    createInfo.setImageExtent(extent);
    createInfo.setImageArrayLayers(1);
    createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };
    if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndexCount(2);
        createInfo.setPQueueFamilyIndices(indices);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    createInfo.setPresentMode(presentMode);
    createInfo.setClipped(VK_TRUE);
    createInfo.setOldSwapchain(nullptr);

    swapchainDetails.swapchain = device.createSwapchainKHR(createInfo);
    Log(debug, fmt::terminal_color::bright_green, "swapchain created");

    std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchainDetails.swapchain);
    swapchainDetails.frames.resize(images.size());

    for (size_t i = 0; i < images.size(); ++i) {

        vk::ImageViewCreateInfo createInfo;
        createInfo.setImage(images[i]);
        createInfo.setViewType(vk::ImageViewType::e2D);
        createInfo.setComponents(vk::ComponentSwizzle::eIdentity);

        vk::ImageSubresourceRange range;
        range.setAspectMask(vk::ImageAspectFlagBits::eColor);
        range.setBaseMipLevel(0);
        range.setLevelCount(1);
        range.setBaseArrayLayer(0);
        range.setLayerCount(1);
        createInfo.setSubresourceRange(range);
        createInfo.setFormat(surfaceFormat.format);

        swapchainDetails.frames[i].image = images[i];
        swapchainDetails.frames[i].imageView = device.createImageView(createInfo);
    }
    swapchainDetails.imageFormat = surfaceFormat.format;
    swapchainDetails.extent = extent;
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

void Swapchain::DestroySwapchain()
{
    for (auto& frame : swapchainDetails.frames) {
        device.destroyImageView(frame.imageView);
        device.destroyFramebuffer(frame.framebuffer);
        device.destroyFence(frame.inFlight);
        device.destroySemaphore(frame.imageAvailable);
        device.destroySemaphore(frame.renderFinished);
    }

    device.destroySwapchainKHR(swapchainDetails.swapchain);
}

Swapchain::~Swapchain()
{
    DestroySwapchain();
}