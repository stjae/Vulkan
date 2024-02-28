#include "device.h"

Device::Device()
{
    PickPhysicalDevice();
    instance_.CreateSurface();
    FindQueueFamilies(Instance::GetBundle().surface);

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    // enable argument buffers
    MVKConfiguration mvkConfig;
    size_t configurationSize = sizeof(MVKConfiguration);
    vkGetMoltenVKConfigurationMVK(Instance::GetBundle().instance, &mvkConfig, &configurationSize);
    mvkConfig.useMetalArgumentBuffers = MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS_ALWAYS;
    vkSetMoltenVKConfigurationMVK(Instance::GetBundle().instance, &mvkConfig, &configurationSize);
#endif

    vk::PhysicalDeviceVulkan12Features features12;
    features12.descriptorIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    deviceBundle_.physicalDevice.getFeatures(&supportedFeatures);
    Device::physicalDeviceLimits = deviceBundle_.physicalDevice.getProperties().limits;
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(instance_.instanceLayers.size()), instance_.instanceLayers.data(), static_cast<uint32_t>(deviceExtensions_.size()), deviceExtensions_.data(), &supportedFeatures, &features12);

    deviceBundle_.device = deviceBundle_.physicalDevice.createDevice(deviceCreateInfo);

    deviceBundle_.graphicsQueue = deviceBundle_.device.getQueue(deviceBundle_.graphicsComputeFamilyIndex.value(), 0);
    deviceBundle_.computeQueue = deviceBundle_.device.getQueue(deviceBundle_.graphicsComputeFamilyIndex.value(), 0);
    deviceBundle_.presentQueue = deviceBundle_.device.getQueue(deviceBundle_.presentFamilyIndex.value(), 0);
}

void Device::PickPhysicalDevice()
{
    auto deviceList = Instance::GetBundle().instance.enumeratePhysicalDevices();

    for (auto& device : deviceList) {
        if (IsDeviceSuitable(device)) {
            deviceBundle_.physicalDevice = device;
            break;
        }
    }

    if (deviceBundle_.physicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(debug, fmt::terminal_color::white, "physical device: {}", deviceBundle_.physicalDevice.getProperties().deviceName);
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice)
{
#if defined(__APPLE__)
    deviceExtensions_.push_back("VK_KHR_portability_subset");
#endif
    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

    std::set<std::string>
        ExtensionSets(deviceExtensions_.begin(), deviceExtensions_.end());

    for (vk::ExtensionProperties& extension : vkPhysicalDevice.enumerateDeviceExtensionProperties()) {
        ExtensionSets.erase(extension.extensionName);
    }

    return ExtensionSets.empty();
}

void Device::FindQueueFamilies(const VkSurfaceKHR& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = deviceBundle_.physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
            deviceBundle_.graphicsComputeFamilyIndex = i;

            Log(debug, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (deviceBundle_.physicalDevice.getSurfaceSupportKHR(i, surface)) {
            deviceBundle_.presentFamilyIndex = i;

            Log(debug, fmt::terminal_color::white, "present queue family index: {}", i);
        }

        if (deviceBundle_.graphicsComputeFamilyIndex.has_value() && deviceBundle_.presentFamilyIndex.has_value()) {
            break;
        }

        i++;
    }

    if (!(deviceBundle_.graphicsComputeFamilyIndex.has_value() && deviceBundle_.presentFamilyIndex.has_value())) {
        spdlog::error("device is not suitable for required queue family");
    }
}

void Device::SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueInfos)
{
    // In case queue families have different indices
    std::vector<uint32_t> uniqueIndices;

    uniqueIndices.push_back(deviceBundle_.graphicsComputeFamilyIndex.value());
    if (deviceBundle_.graphicsComputeFamilyIndex.value() != deviceBundle_.presentFamilyIndex.value()) {
        uniqueIndices.push_back(deviceBundle_.presentFamilyIndex.value());
    }

    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex : uniqueIndices) {
        deviceQueueInfos.push_back(
            vk::DeviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority));
    }
}

Device::~Device()
{
    deviceBundle_.device.destroy();
}