#ifndef DEVICE_H
#define DEVICE_H

#include "instance.h"

namespace vkn {
struct DeviceBundle
{
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
    vk::Queue graphicsQueue;
    vk::Queue computeQueue;
    vk::Queue presentQueue;
    std::optional<uint32_t> graphicsFamilyIndex;
    std::optional<uint32_t> computeFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;
};
class Device
{
    Instance instance_;
    std::vector<const char*> deviceExtensions_;
    inline static DeviceBundle deviceBundle_;

    void FindQueueFamilies(const VkSurfaceKHR& surface);
    void SetDeviceQueueCreateInfo(std::__1::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);

public:
    inline static vk::PhysicalDeviceLimits physicalDeviceLimits;

    Device();
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice);
    ~Device();

    static const DeviceBundle& GetBundle() { return deviceBundle_; }
};
} // namespace vkn

#endif