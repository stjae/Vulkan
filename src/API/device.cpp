#include "device.h"

Device::Device()
{
    PickPhysicalDevice();
    instance.CreateSurface();
    FindQueueFamilies();

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    deviceExtensions.push_back("VK_KHR_portability_subset");
#endif

    vk::PhysicalDeviceVulkan12Features features12;
    features12.runtimeDescriptorArray = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    GetPhysicalDevice().getFeatures(&supportedFeatures);
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(instance.GetNumInstanceLayers()), instance.GetInstanceLayers(), static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(), &supportedFeatures, &features12);

    // Get device handle
    GetDevice() = GetPhysicalDevice().createDevice(deviceCreateInfo);

    // Get queue handle
    GetGraphicsQueue() = GetDevice().getQueue(GetQueueFamilyIndices().graphicsFamily.value(), 0);
    GetPresentQueue() = GetDevice().getQueue(GetQueueFamilyIndices().presentFamily.value(), 0);
}

void Device::SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueInfos)
{
    // In case queue families have different indices
    std::vector<uint32_t> uniqueIndices;
    uniqueIndices.push_back(GetQueueFamilyIndices().graphicsFamily.value());
    if (GetQueueFamilyIndices().graphicsFamily.value() != GetQueueFamilyIndices().presentFamily.value()) {
        uniqueIndices.push_back(GetQueueFamilyIndices().presentFamily.value());
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
    auto devices = Instance::GetInstance().enumeratePhysicalDevices();

    for (auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            GetPhysicalDevice() = device;
            break;
        }
    }

    if (GetPhysicalDevice() == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(debug, fmt::terminal_color::white, "physical device: {}", GetPhysicalDevice().getProperties().deviceName);
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice)
{
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    std::set<std::string> ExtensionSets(deviceExtensions.begin(), deviceExtensions.end());

    for (vk::ExtensionProperties& extension : vkPhysicalDevice.enumerateDeviceExtensionProperties()) {
        ExtensionSets.erase(extension.extensionName);
    }

    return ExtensionSets.empty() ? true : false;
}

void Device::FindQueueFamilies()
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = GetPhysicalDevice().getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            GetQueueFamilyIndices().graphicsFamily = i;

            Log(debug, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (GetPhysicalDevice().getSurfaceSupportKHR(i, Instance::GetSurface())) {
            GetQueueFamilyIndices().presentFamily = i;

            Log(debug, fmt::terminal_color::white, "present queue family index: {}", i);
        }

        if (GetQueueFamilyIndices().IsComplete()) {
            break;
        }

        i++;
    }

    if (!GetQueueFamilyIndices().IsComplete()) {
        spdlog::error("device is not suitable for required queue family");
    }
}

Device::~Device()
{
    GetDevice().destroy();
}