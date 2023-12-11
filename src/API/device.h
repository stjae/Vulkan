#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "instance.h"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class Device
{

    Instance instance;
    std::vector<const char*> deviceExtensions;

public:
    Device();
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice);
    void FindQueueFamilies();
    static vk::Device& GetDevice()
    {
        static vk::Device vkDevice;
        return vkDevice;
    }
    static vk::PhysicalDevice& GetPhysicalDevice()
    {
        static vk::PhysicalDevice vkPhysicalDevice;
        return vkPhysicalDevice;
    }
    static QueueFamilyIndices& GetQueueFamilyIndices()
    {
        static QueueFamilyIndices queueFamilyIndices;
        return queueFamilyIndices;
    }
    static vk::Queue& GetGraphicsQueue()
    {
        static vk::Queue vkGraphicsQueue;
        return vkGraphicsQueue;
    }
    static vk::Queue& GetPresentQueue()
    {
        static vk::Queue vkPresentQueue;
        return vkPresentQueue;
    }

    ~Device();
};

#endif