#include "swapchain.h"

void Swapchain::QuerySwapchainSupportDetails()
{
    swapchainSupportDetails.capabilities = Device::physicalDevice.getSurfaceCapabilitiesKHR(Instance::surface);
    swapchainSupportDetails.formats = Device::physicalDevice.getSurfaceFormatsKHR(Instance::surface);
    swapchainSupportDetails.presentModes = Device::physicalDevice.getSurfacePresentModesKHR(Instance::surface);
    auto& capabilities = swapchainSupportDetails.capabilities;
    auto& formats = swapchainSupportDetails.formats;
    auto& presentModes = swapchainSupportDetails.presentModes;

    if (debug) {
        spdlog::info("printing queries for surface supports..");

        spdlog::info("current surface extent width: {}", capabilities.currentExtent.width);
        spdlog::info("current surface extent height: {}", capabilities.currentExtent.height);

        // for(auto& format : formats) {
        //     spdlog::info("supported surface color space: {}", vk::to_string(format.colorSpace));
        //     spdlog::info("supported surface format: {}", vk::to_string(format.format));
        // }

        for (auto& mode : presentModes) {
            spdlog::info("supported present mode: {}", vk::to_string(mode));
        }
    }
}

void Swapchain::CreateSwapchain()
{
    auto& capabilities = swapchainSupportDetails.capabilities;

    auto surfaceFormat = ChooseSurfaceFormat();
    auto presentMode = ChoosePresentMode();
    auto extent = ChooseExtent();

    uint32_t imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setSurface(Instance::surface);
    createInfo.setMinImageCount(imageCount);
    createInfo.setImageFormat(surfaceFormat.format);
    createInfo.setImageColorSpace(surfaceFormat.colorSpace);
    createInfo.setImageExtent(extent);
    createInfo.setImageArrayLayers(1);
    createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t indices[] = { Device::queueFamilyIndices.graphicsFamily.value(), Device::queueFamilyIndices.presentFamily.value() };
    if (Device::queueFamilyIndices.graphicsFamily.value() != Device::queueFamilyIndices.presentFamily.value()) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndexCount(2);
        createInfo.setPQueueFamilyIndices(indices);
    } else {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    createInfo.setPresentMode(presentMode);
    createInfo.setClipped(VK_TRUE);
    createInfo.setOldSwapchain(nullptr);

    swapchainDetails.swapchain = Device::device.createSwapchainKHR(createInfo);
    std::vector<vk::Image> images = Device::device.getSwapchainImagesKHR(swapchainDetails.swapchain);
    swapchainDetails.frames.resize(images.size());

    for (size_t i = 0; i < images.size(); ++i) {
        swapchainDetails.frames[i].image = images[i];

        vk::ImageViewCreateInfo createInfo;
        createInfo.setViewType(vk::ImageViewType::e2D);
        createInfo.setComponents(vk::ComponentSwizzle::eIdentity);
        //createInfo.setSubresourceRange();

    }
    swapchainDetails.format = surfaceFormat.format;
    swapchainDetails.extent = extent;

    if (debug) {
        spdlog::info("swapchain created");
    }
}

vk::SurfaceFormatKHR Swapchain::ChooseSurfaceFormat()
{
    for (auto& format : swapchainSupportDetails.formats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            if (debug) {
                spdlog::info("chosen surface pixel format: {}", vk::to_string(format.format));
                spdlog::info("chosen surface color space: {}", vk::to_string(format.colorSpace));
            }
            return format;
        }
    }

    if (debug) {
        spdlog::info("chosen surface pixel format: {}", vk::to_string(swapchainSupportDetails.formats[0].format));
        spdlog::info("chosen surface color space: {}", vk::to_string(swapchainSupportDetails.formats[0].colorSpace));
    }
    return swapchainSupportDetails.formats[0];
}

vk::PresentModeKHR Swapchain::ChoosePresentMode()
{
    for (auto& presentMode : swapchainSupportDetails.presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            return presentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::ChooseExtent()
{
    auto& capabilities = swapchainSupportDetails.capabilities;

    // extent is set
    if (capabilities.currentExtent.width != UINT32_MAX) {
        if (debug) {
            spdlog::info("no change in extent size");
        }
        return capabilities.currentExtent;
    } else {
        // extent is not set
        int width, height;
        glfwGetFramebufferSize(Window::window, &width, &height);

        vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        if (extent.width > capabilities.maxImageExtent.width) {

            extent.width = capabilities.maxImageExtent.width;
            if (debug) {
                spdlog::info("extent width is clamped");
            }
        }
        if (extent.height > capabilities.maxImageExtent.height) {

            extent.height = capabilities.maxImageExtent.height;
            if (debug) {
                spdlog::info("extent height is clamped");
            }
        }

        return extent;
    }
}

Swapchain::~Swapchain()
{
    Device::device.destroySwapchainKHR(swapchainDetails.swapchain);
}

SwapchainSupportDetails Swapchain::swapchainSupportDetails;
SwapchainDetails Swapchain::swapchainDetails;
