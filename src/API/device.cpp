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

Device::~Device()
{
    device.destroy();
}

vk::PhysicalDevice Device::physicalDevice;
std::vector<const char*> Device::extensions;
QueueFamilyIndices Device::queueFamilyIndices;
vk::Device Device::device;
vk::Queue Device::graphicsQueue;
vk::Queue Device::presentQueue;