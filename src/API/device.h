#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "instance.h"

struct QueueFamilyIndices {

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Device
{
public:
    // Device related method
    Device(GLFWwindow* window);
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);

    // Physical Device related method
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice& device);
    void FindQueueFamilies();

    ~Device();

    Instance instance;

    vk::PhysicalDevice vkPhysicalDevice;
    std::vector<const char*> deviceExtensions;
    QueueFamilyIndices queueFamilyIndices;
    vk::Device vkDevice;
    vk::Queue vkGraphicsQueue;
    vk::Queue vkPresentQueue;

    vk::DescriptorPool imGuiDescriptorPool;
};

#endif