#include "device.h"

void Device::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    auto devices = Instance::Get().enumeratePhysicalDevices();

    for (auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            PhysicalDevice() = device;
            break;
        }
    }

    if (PhysicalDevice() == nullptr) {
        spdlog::error("no suitable device found");
    } else if (debug) {
        spdlog::info("physical device: {}", PhysicalDevice().getProperties().deviceName);
    }
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice& device)
{
    const std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    std::set<std::string> requiredExtensionSets(requiredExtensions.begin(), requiredExtensions.end());

    for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
        requiredExtensionSets.erase(extension.extensionName);
    }

    return requiredExtensionSets.empty() ? true : false;
}

void Device::FindQueueFamilies()
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = Device::PhysicalDevice().getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            QueueFamilyIndices().graphicsFamily = i;

            if (debug) {
                spdlog::info("queue family index for graphics is {}", i);
            }
        }

        if (QueueFamilyIndices().IsComplete()) {
            break;
        }

        i++;
    }

    if (!QueueFamilyIndices().IsComplete()) {
        spdlog::error("device is not suitable for required queue family");
    }
}

void Device::CreateLogicalDevice()
{
    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo deviceQueueInfo;
    deviceQueueInfo.setQueueFamilyIndex(QueueFamilyIndices().graphicsFamily.value());
    deviceQueueInfo.setQueueCount(1);
    deviceQueueInfo.setPQueuePriorities(&queuePriority);

    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceInfo;
    deviceInfo.setQueueCreateInfos(deviceQueueInfo);
    deviceInfo.setQueueCreateInfoCount(1);
    deviceInfo.setEnabledLayerCount(Instance::Layers().size());
    deviceInfo.setPpEnabledLayerNames(Instance::Layers().data());

    std::vector<const char*> extensions = { "VK_KHR_portability_subset" };
    deviceInfo.setEnabledExtensionCount(extensions.size());
    deviceInfo.setPpEnabledExtensionNames(extensions.data());
    deviceInfo.setPEnabledFeatures(&deviceFeatures);

    Device::Get() = PhysicalDevice().createDevice(deviceInfo);
}

Device::~Device()
{
    Device::Get().destroy();
}

// getter
vk::PhysicalDevice& Device::PhysicalDevice()
{
    static vk::PhysicalDevice physicalDevice;
    return physicalDevice;
}

vkStruct::QueueFamilyIndices& Device::QueueFamilyIndices()
{
    static vkStruct::QueueFamilyIndices queueFamilyIndices;
    return queueFamilyIndices;
}

vk::Device& Device::Get()
{
    static vk::Device device;
    return device;
}
