#include "device.h"

vkn::Device::Device()
{
    PickPhysicalDevice();
    instance_.CreateSurface();
    FindQueueFamilies(vkn::Instance::GetBundle().surface);

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    // enable argument buffers
    MVKConfiguration mvkConfig;
    size_t configurationSize = sizeof(MVKConfiguration);
    vkGetMoltenVKConfigurationMVK(vkn::Instance::GetBundle().instance, &mvkConfig, &configurationSize);
    mvkConfig.useMetalArgumentBuffers = MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS_ALWAYS;
    vkSetMoltenVKConfigurationMVK(vkn::Instance::GetBundle().instance, &mvkConfig, &configurationSize);
#endif

    vk::PhysicalDeviceVulkan12Features features12;
    features12.descriptorIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    features12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    deviceBundle_.physicalDevice.getFeatures(&supportedFeatures);
    Device::physicalDeviceLimits = deviceBundle_.physicalDevice.getProperties().limits;
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(instance_.instanceLayers_.size()), instance_.instanceLayers_.data(), static_cast<uint32_t>(deviceExtensions_.size()), deviceExtensions_.data(), &supportedFeatures, &features12);

    deviceBundle_.device = deviceBundle_.physicalDevice.createDevice(deviceCreateInfo);

    deviceBundle_.graphicsQueue = deviceBundle_.device.getQueue(deviceBundle_.graphicsFamilyIndex.value(), 0);
    deviceBundle_.computeQueue = deviceBundle_.device.getQueue(deviceBundle_.computeFamilyIndex.value(), 0);
    deviceBundle_.presentQueue = deviceBundle_.device.getQueue(deviceBundle_.presentFamilyIndex.value(), 0);
}

void vkn::Device::PickPhysicalDevice()
{
    auto deviceList = vkn::Instance::GetBundle().instance.enumeratePhysicalDevices();

    for (auto& device : deviceList) {
        if (IsDeviceSuitable(device)) {
            deviceBundle_.physicalDevice = device;
            break;
        }
    }

    if (deviceBundle_.physicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(debugMode, fmt::terminal_color::white, "physical device: {}", deviceBundle_.physicalDevice.getProperties().deviceName);
}

bool vkn::Device::IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice)
{
#if defined(__APPLE__)
    deviceExtensions_.push_back("VK_KHR_portability_subset");
#endif
    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    std::set<std::string> extensionSets(deviceExtensions_.begin(), deviceExtensions_.end());

    for (vk::ExtensionProperties& extension : vkPhysicalDevice.enumerateDeviceExtensionProperties()) {
        extensionSets.erase(extension.extensionName);
    }

    return extensionSets.empty();
}

void vkn::Device::FindQueueFamilies(const VkSurfaceKHR& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = deviceBundle_.physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            deviceBundle_.graphicsFamilyIndex = i;

            Log(debugMode, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
            deviceBundle_.computeFamilyIndex = i;

            Log(debugMode, fmt::terminal_color::white, "compute queue family index: {}", i);
        }

        if (deviceBundle_.physicalDevice.getSurfaceSupportKHR(i, surface)) {
            deviceBundle_.presentFamilyIndex = i;

            Log(debugMode, fmt::terminal_color::white, "present queue family index: {}", i);
        }

        if (deviceBundle_.graphicsFamilyIndex.has_value() && deviceBundle_.computeFamilyIndex.has_value() && deviceBundle_.presentFamilyIndex.has_value()) {
            break;
        }

        i++;
    }

    if (!(deviceBundle_.graphicsFamilyIndex.has_value() && deviceBundle_.computeFamilyIndex.has_value() && deviceBundle_.presentFamilyIndex.has_value())) {
        spdlog::error("device is not suitable for required queue family");
    }
}

void vkn::Device::SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueInfos)
{
    // In case queue families have different indices
    std::vector<uint32_t> uniqueIndices;

    uniqueIndices.push_back(deviceBundle_.graphicsFamilyIndex.value());
    if (deviceBundle_.graphicsFamilyIndex.value() != deviceBundle_.computeFamilyIndex.value()) {
        uniqueIndices.push_back(deviceBundle_.computeFamilyIndex.value());
    }
    if (deviceBundle_.graphicsFamilyIndex.value() != deviceBundle_.presentFamilyIndex.value()) {
        uniqueIndices.push_back(deviceBundle_.presentFamilyIndex.value());
    }

    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex : uniqueIndices) {
        deviceQueueInfos.push_back(
            vk::DeviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority));
    }
}

vkn::Device::~Device()
{
    deviceBundle_.device.destroy();
}