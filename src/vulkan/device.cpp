#include "device.h"

namespace vkn {
Device::Device()
{
    PickPhysicalDevice();
    s_bundle.maxSampleCount = GetSupportedMaxSampleCount();
    m_instance.CreateSurface();
    FindQueueFamilies(Instance::GetSurface());

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

    vk::PhysicalDeviceVulkan12Features features12;
    features12.descriptorIndexing = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    s_bundle.physicalDevice.getFeatures(&supportedFeatures);
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(m_instance.m_layers.size()), m_instance.m_layers.data(), static_cast<uint32_t>(m_deviceExtensions.size()), m_deviceExtensions.data(), &supportedFeatures, &features12);

    s_bundle.device = s_bundle.physicalDevice.createDevice(deviceCreateInfo);

    s_bundle.graphicsQueue = s_bundle.device.getQueue(s_bundle.graphicsFamilyIndex.value(), 0);
    s_bundle.computeQueue = s_bundle.device.getQueue(s_bundle.computeFamilyIndex.value(), 0);
    s_bundle.presentQueue = s_bundle.device.getQueue(s_bundle.presentFamilyIndex.value(), 0);
}

void Device::PickPhysicalDevice()
{
    uint32_t deviceCount;
    CHECK_RESULT(Instance::GetInstance().enumeratePhysicalDevices(&deviceCount, nullptr));
    std::vector<vk::PhysicalDevice> physicalDeviceList(deviceCount);
    CHECK_RESULT(Instance::GetInstance().enumeratePhysicalDevices(&deviceCount, physicalDeviceList.data()));

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    std::set<std::string> extensionSets(deviceExtensions.begin(), deviceExtensions.end());

    for (auto& physicalDevice : physicalDeviceList) {
        bool isDeviceSupported = [&]() {
            for (auto& supportedExtension : physicalDevice.enumerateDeviceExtensionProperties()) {
                extensionSets.erase(supportedExtension.extensionName);
            }
            return extensionSets.empty();
        }();
        if (isDeviceSupported) {
            s_bundle.physicalDevice = physicalDevice;
            break;
        }
    }

    if (!s_bundle.physicalDevice) {
        spdlog::error("no suitable device was found");
        exit(1);
    }

    m_deviceExtensions = deviceExtensions;
}

vk::SampleCountFlagBits Device::GetSupportedMaxSampleCount()
{
    vk::SampleCountFlags sampleCount = s_bundle.physicalDevice.getProperties().limits.framebufferColorSampleCounts & s_bundle.physicalDevice.getProperties().limits.framebufferDepthSampleCounts;
    if (sampleCount & vk::SampleCountFlagBits::e64)
        return vk::SampleCountFlagBits::e64;
    if (sampleCount & vk::SampleCountFlagBits::e32)
        return vk::SampleCountFlagBits::e32;
    if (sampleCount & vk::SampleCountFlagBits::e16)
        return vk::SampleCountFlagBits::e16;
    if (sampleCount & vk::SampleCountFlagBits::e8)
        return vk::SampleCountFlagBits::e8;
    if (sampleCount & vk::SampleCountFlagBits::e4)
        return vk::SampleCountFlagBits::e4;
    if (sampleCount & vk::SampleCountFlagBits::e2)
        return vk::SampleCountFlagBits::e2;
    return vk::SampleCountFlagBits::e1;
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
