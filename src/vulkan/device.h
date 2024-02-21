#ifndef DEVICE_H
#define DEVICE_H

#include "instance.h"

struct DeviceBundle
{
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
    vk::Queue graphicsQueue;
    vk::Queue computeQueue;
    vk::Queue presentQueue;
    std::optional<uint32_t> graphicsComputeFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;
};

class Device
{
    Instance instance_;
    std::vector<const char*> deviceExtensions_;
    inline static DeviceBundle deviceBundle_;

    void FindQueueFamilies(const VkSurfaceKHR& surface);
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);

public:
    inline static vk::PhysicalDeviceLimits physicalDeviceLimits;

    Device();
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice);
    ~Device();

    static const DeviceBundle& GetBundle() { return deviceBundle_; }
};
#endif