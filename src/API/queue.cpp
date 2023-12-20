#include "queue.h"

void Queue::FindQueueFamilies(const vk::PhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    int i = 0;
    for (auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily_ = i;

            Log(debug, fmt::terminal_color::white, "graphics queue family index: {}", i);
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
            presentFamily_ = i;

            Log(debug, fmt::terminal_color::white, "present queue family index: {}", i);
        }

        if (graphicsFamily_.has_value() && presentFamily_.has_value()) {
            break;
        }

        i++;
    }

    if (!(graphicsFamily_.has_value() && presentFamily_.has_value())) {
        spdlog::error("device is not suitable for required queue family");
    }
}

void Queue::SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueInfos)
{
    // In case queue families have different indices
    std::vector<uint32_t> uniqueIndices;

    uniqueIndices.push_back(graphicsFamily_.value());
    if (graphicsFamily_.value() != presentFamily_.value()) {
        uniqueIndices.push_back(presentFamily_.value());
    }

    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex : uniqueIndices) {
        deviceQueueInfos.push_back(
            vk::DeviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority));
    }
}