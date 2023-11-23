#include "device.h"

Device::Device(GLFWwindow* window)
{
    PickPhysicalDevice();
    instance.CreateSurface(window);
    FindQueueFamilies();

    std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
    SetDeviceQueueCreateInfo(deviceQueueCreateInfos);

#if defined(__APPLE__)
    deviceExtensions.push_back("VK_KHR_portability_subset");
#endif

    vk::PhysicalDeviceVulkan12Features features12;
    features12.runtimeDescriptorArray = VK_TRUE;

    vk::PhysicalDeviceFeatures supportedFeatures;
    vkPhysicalDevice.getFeatures(&supportedFeatures);
    vk::DeviceCreateInfo deviceCreateInfo({}, static_cast<uint32_t>(deviceQueueCreateInfos.size()), deviceQueueCreateInfos.data(), static_cast<uint32_t>(instance.instanceLayers.size()), instance.instanceLayers.data(), static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data(), &supportedFeatures, &features12);

    // Get device handle
    vkDevice = vkPhysicalDevice.createDevice(deviceCreateInfo);

    // Get queue handle
    vkGraphicsQueue = vkDevice.getQueue(queueFamilyIndices.graphicsFamily.value(), 0);
    vkPresentQueue = vkDevice.getQueue(queueFamilyIndices.presentFamily.value(), 0);
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
    auto devices = instance.vkInstance.enumeratePhysicalDevices();

    for (auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            vkPhysicalDevice = device;
            break;
        }
    }

    if (vkPhysicalDevice == nullptr) {
        spdlog::error("no suitable device found");
    }
    Log(debug, fmt::terminal_color::white, "physical device: {}", vkPhysicalDevice.getProperties().deviceName);
}

bool Device::IsDeviceSuitable(const vk::PhysicalDevice& device)
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
    std::vector<vk::QueueFamilyProperties> queueFamilies = vkPhysicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsFamily = i;

            Log(debug, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (vkPhysicalDevice.getSurfaceSupportKHR(i, instance.vkSurface)) {
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
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    vkDevice.destroyDescriptorPool(imGuiDescriptorPool);
    ImGui::DestroyContext();

    vkDevice.destroy();
}