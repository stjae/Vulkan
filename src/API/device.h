#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "instance.h"

class Device
{
public:
    ~Device();

    // Device related method
    void CreateDevice();
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);

    // Physical Device related method
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice& device);
    void FindQueueFamilies();
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

inline vk::PhysicalDevice physicalDevice;
inline std::vector<const char*> deviceExtensions;
inline QueueFamilyIndices queueFamilyIndices;
inline vk::Device device;
inline vk::Queue graphicsQueue;
inline vk::Queue presentQueue;

#endif