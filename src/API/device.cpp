#include "device.h"

Device::Device()
{
    PickPhysicalDevice();
    instance_.CreateSurface();
    queue_.FindQueueFamilies(handle_.physicalDevice, Instance::GetHandle().surface);

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    queue_.SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    // enable argument buffers
    MVKConfiguration mvkConfig;
    size_t configurationSize = sizeof(MVKConfiguration);
    vkGetMoltenVKConfigurationMVK(Instance::GetHandle().instance, &mvkConfig, &configurationSize);
    mvkConfig.useMetalArgumentBuffers = MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS_ALWAYS;
    vkSetMoltenVKConfigurationMVK(Instance::GetHandle().instance, &mvkConfig, &configurationSize);

    deviceExtensions_.push_back("VK_KHR_portability_subset");
#endif

    deviceExtensions_.push_back("VK_EXT_descriptor_indexing");
    vk::PhysicalDeviceVulkan12Features features12;
    features12.descriptorIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    handle_.physicalDevice.getFeatures(&supportedFeatures);
    Device::limits = handle_.physicalDevice.getProperties().limits;
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(instance_.GetNumInstanceLayers()), instance_.GetInstanceLayers(), static_cast<uint32_t>(deviceExtensions_.size()), deviceExtensions_.data(), &supportedFeatures, &features12);

    // Set device handle
    handle_.device = handle_.physicalDevice.createDevice(deviceCreateInfo);

    // Set queue handle
    vk::Queue graphicsQueue = handle_.device.getQueue(Queue::GetGraphicsQueueFamilyIndex(), 0);
    vk::Queue presentQueue = handle_.device.getQueue(Queue::GetGraphicsQueueFamilyIndex(), 0);
    queue_.SetHandle(graphicsQueue, presentQueue);
}

void Device::PickPhysicalDevice()
{
    auto deviceList = Instance::GetHandle().instance.enumeratePhysicalDevices();

    for (auto& device : deviceList) {
        if (IsDeviceSuitable(device)) {
            handle_.physicalDevice = device;
            break;
        }
    }

    if (handle_.physicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(debug, fmt::terminal_color::white, "physical device: {}", handle_.physicalDevice.getProperties().deviceName);
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice)
{
    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    std::set<std::string> ExtensionSets(deviceExtensions_.begin(), deviceExtensions_.end());

    for (vk::ExtensionProperties& extension : vkPhysicalDevice.enumerateDeviceExtensionProperties()) {
        ExtensionSets.erase(extension.extensionName);
    }

    return ExtensionSets.empty();
}

Device::~Device()
{
    handle_.device.destroy();
}