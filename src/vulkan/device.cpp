#include "device.h"

namespace vkn {
Device::Device()
{
    PickPhysicalDevice();
    m_instance.CreateSurface();
    FindQueueFamilies(Instance::GetSurface());

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    // enable argument buffers
    MVKConfiguration mvkConfig;
    size_t configurationSize = sizeof(MVKConfiguration);
    vkGetMoltenVKConfigurationMVK(Instance::GetInstance(), &mvkConfig, &configurationSize);
    mvkConfig.useMetalArgumentBuffers = MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS_ALWAYS;
    vkSetMoltenVKConfigurationMVK(Instance::GetInstance(), &mvkConfig, &configurationSize);
#endif

    vk::PhysicalDeviceVulkan12Features features12;
    features12.descriptorIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    features12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    s_bundle.physicalDevice.getFeatures(&supportedFeatures);
    Device::physicalDeviceLimits = s_bundle.physicalDevice.getProperties().limits;
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(m_instance.m_layers.size()), m_instance.m_layers.data(), static_cast<uint32_t>(m_deviceExtensions.size()), m_deviceExtensions.data(), &supportedFeatures, &features12);

    s_bundle.device = s_bundle.physicalDevice.createDevice(deviceCreateInfo);

    s_bundle.graphicsQueue = s_bundle.device.getQueue(s_bundle.graphicsFamilyIndex.value(), 0);
    s_bundle.computeQueue = s_bundle.device.getQueue(s_bundle.computeFamilyIndex.value(), 0);
    s_bundle.presentQueue = s_bundle.device.getQueue(s_bundle.presentFamilyIndex.value(), 0);
}

void Device::PickPhysicalDevice()
{
    auto deviceList = Instance::GetInstance().enumeratePhysicalDevices();

    for (auto& device : deviceList) {
        if (IsDeviceSuitable(device)) {
            s_bundle.physicalDevice = device;
            break;
        }
    }

    if (s_bundle.physicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(DEBUG, fmt::terminal_color::white, "physical device: {}", std::string(s_bundle.physicalDevice.getProperties().deviceName));
}

bool Device::IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice)
{
#if defined(__APPLE__)
    m_deviceExtensions.push_back("VK_KHR_portability_subset");
#endif
    m_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    m_deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    m_deviceExtensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);

    std::set<std::string> extensionSets(m_deviceExtensions.begin(), m_deviceExtensions.end());

    for (vk::ExtensionProperties& extension : vkPhysicalDevice.enumerateDeviceExtensionProperties()) {
        extensionSets.erase(extension.extensionName);
    }

    return extensionSets.empty();
}

void Device::FindQueueFamilies(const VkSurfaceKHR& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = s_bundle.physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            s_bundle.graphicsFamilyIndex = i;

            Log(DEBUG, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
            s_bundle.computeFamilyIndex = i;

            Log(DEBUG, fmt::terminal_color::white, "compute queue family index: {}", i);
        }

        if (s_bundle.physicalDevice.getSurfaceSupportKHR(i, surface)) {
            s_bundle.presentFamilyIndex = i;

            Log(DEBUG, fmt::terminal_color::white, "present queue family index: {}", i);
        }

        if (s_bundle.graphicsFamilyIndex.has_value() && s_bundle.computeFamilyIndex.has_value() && s_bundle.presentFamilyIndex.has_value()) {
            break;
        }

        i++;
    }

    if (!(s_bundle.graphicsFamilyIndex.has_value() && s_bundle.computeFamilyIndex.has_value() && s_bundle.presentFamilyIndex.has_value())) {
        spdlog::error("device is not suitable for required queue family");
    }
}

void Device::SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueInfos)
{
    // In case queue families have different indices
    std::vector<uint32_t> uniqueIndices;

    uniqueIndices.push_back(s_bundle.graphicsFamilyIndex.value());
    if (s_bundle.graphicsFamilyIndex.value() != s_bundle.computeFamilyIndex.value()) {
        uniqueIndices.push_back(s_bundle.computeFamilyIndex.value());
    }
    if (s_bundle.graphicsFamilyIndex.value() != s_bundle.presentFamilyIndex.value()) {
        uniqueIndices.push_back(s_bundle.presentFamilyIndex.value());
    }

    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex : uniqueIndices) {
        deviceQueueInfos.push_back(
            vk::DeviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority));
    }
}

Device::~Device()
{
    s_bundle.device.destroy();
}
}; // namespace vkn
