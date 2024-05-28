#ifndef DEVICE_H
#define DEVICE_H

#include "instance.h"
#include <optional>
#include <set>

namespace vkn {
class Device
{
    struct Bundle
    {
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
        vk::Queue graphicsQueue;
        vk::Queue computeQueue;
        vk::Queue presentQueue;
        std::optional<uint32_t> graphicsFamilyIndex;
        std::optional<uint32_t> computeFamilyIndex;
        std::optional<uint32_t> presentFamilyIndex;
    } inline static s_bundle;
    Instance m_instance;
    std::vector<const char*> m_deviceExtensions;

    void FindQueueFamilies(const VkSurfaceKHR& surface);
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice);

public:
    inline static std::vector<vk::SubmitInfo> s_submitInfos;

    Device();
    ~Device();
    static const Bundle& Get() { return s_bundle; }
};
} // namespace vkn

#endif