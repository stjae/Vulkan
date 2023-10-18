#include "device.h"

void Device::CreateDevice()
{
    PickPhysicalDevice();
    FindQueueFamilies();

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    deviceExtensions.push_back("VK_KHR_portability_subset");
#endif
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(instanceLayers.size()), instanceLayers.data(), static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data());

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
            vk::DeviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority));
    }
}

void Device::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    auto devices = instance.enumeratePhysicalDevices();

    for (auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(debug, fmt::terminal_color::white, "physical device: {}", physicalDevice.getProperties().deviceName);
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice& device)
{
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    std::set<std::string> ExtensionSets(deviceExtensions.begin(), deviceExtensions.end());

    for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
        ExtensionSets.erase(extension.extensionName);
    }

    return ExtensionSets.empty() ? true : false;
}

void Device::FindQueueFamilies()
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;

            Log(debug, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
            queueFamilyIndices.presentFamily = i;

            Log(debug, fmt::terminal_color::white, "present queue family index: {}", i);
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

Device::~Device()
{
    device.destroy();
}