#include "device.h"

void Device::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    auto devices = Instance::instance.enumeratePhysicalDevices();

    for (auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    } else if (debug) {
        spdlog::info("physical device: {}", physicalDevice.getProperties().deviceName);
    }
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice& device)
{
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    std::set<std::string> ExtensionSets(extensions.begin(), extensions.end());

    for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
        ExtensionSets.erase(extension.extensionName);
    }

    return ExtensionSets.empty() ? true : false;
}

void Device::FindQueueFamilies()
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = Device::physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;

            if (debug) {
                spdlog::info("graphics queue family index: {}", i);
            }
        }

        if (physicalDevice.getSurfaceSupportKHR(i, Instance::surface)) {
            queueFamilyIndices.presentFamily = i;

            if (debug) {
                spdlog::info("present queue family index: {}", i);
            }
        }

        if (queueFamilyIndices.IsComplete()) {
            break;
        }

        i++;
    }

    if (!queueFamilyIndices.IsComplete()) {
        spdlog::error("device is not suitable for required queue family");
    }
}

void Device::CreateDevice()
{
    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    Device::SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

    vk::DeviceCreateInfo deviceCreateInfo;
    Device::SetDeviceCreateInfo(deviceCreateInfo, deviceQueueCreateInfos);
    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);

    // Get device handle
    device = physicalDevice.createDevice(deviceCreateInfo);

    // Get queue handle
    graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(queueFamilyIndices.presentFamily.value(), 0);
}

void Device::SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueInfos)
{
    // In case queue families have different indices
    std::vector<uint32_t> uniqueIndices;
    uniqueIndices.push_back(queueFamilyIndices.graphicsFamily.value());
    if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
        uniqueIndices.push_back(queueFamilyIndices.presentFamily.value());
    }

    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex : uniqueIndices) {
        deviceQueueInfos.push_back(
            vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndex, 1, &queuePriority));
    }
}

void Device::SetDeviceCreateInfo(vk::DeviceCreateInfo& deviceCreateInfo, std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos)
{
    deviceCreateInfo.setPQueueCreateInfos(deviceQueueCreateInfos.data());
    deviceCreateInfo.setQueueCreateInfoCount(deviceQueueCreateInfos.size());
    deviceCreateInfo.setEnabledLayerCount(Instance::layers.size());
    deviceCreateInfo.setPpEnabledLayerNames(Instance::layers.data());

#if defined(__APPLE__)
    extensions.push_back("VK_KHR_portability_subset");
#endif
    deviceCreateInfo.setEnabledExtensionCount(extensions.size());
    deviceCreateInfo.setPpEnabledExtensionNames(extensions.data());
}

void Device::QuerySwapchainSupportDetails()
{
    swapchainSupportDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(Instance::surface);
    swapchainSupportDetails.formats = physicalDevice.getSurfaceFormatsKHR(Instance::surface);
    swapchainSupportDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(Instance::surface);
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

void Device::CreateSwapchain()
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
    swapchainDetails.images = device.getSwapchainImagesKHR(swapchainDetails.swapchain);
    swapchainDetails.format = surfaceFormat.format;
    swapchainDetails.extent = extent;

    if (debug) {
        spdlog::info("swapchain created");
    }
}

vk::SurfaceFormatKHR Device::ChooseSurfaceFormat()
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

vk::PresentModeKHR Device::ChoosePresentMode()
{
    for (auto& presentMode : swapchainSupportDetails.presentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            return presentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Device::ChooseExtent()
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

Device::~Device()
{
    device.destroySwapchainKHR(swapchainDetails.swapchain);
    device.destroy();
}

vk::PhysicalDevice Device::physicalDevice;
std::vector<const char*> Device::extensions;
QueueFamilyIndices Device::queueFamilyIndices;
vk::Device Device::device;
vk::Queue Device::graphicsQueue;
vk::Queue Device::presentQueue;
SwapchainSupportDetails Device::swapchainSupportDetails;
SwapchainDetails Device::swapchainDetails;