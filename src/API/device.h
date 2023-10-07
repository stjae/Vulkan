#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "instance.h"

struct QueueFamilyIndices;

class Device
{
public:
    ~Device();

    // Physical Device related method
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice& device);
    void FindQueueFamilies();

    // Device related method
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);
    void SetDeviceCreateInfo(vk::DeviceCreateInfo& deviceCreateInfo, std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);
    void CreateDevice();

    // Vulkan object variable
    static vk::PhysicalDevice physicalDevice;
    static std::vector<const char*> extensions;
    static QueueFamilyIndices queueFamilyIndices;
    static vk::Device device;
    static vk::Queue graphicsQueue;
    static vk::Queue presentQueue;
};

struct QueueFamilyIndices {

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    QueueFamilyIndices() {}
    QueueFamilyIndices(const QueueFamilyIndices&) = delete;
    QueueFamilyIndices& operator=(const QueueFamilyIndices&) = delete;
};

#endif