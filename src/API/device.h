#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"

namespace vkStruct
{
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    // std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        // return graphicsFamily.has_value() && presentFamily.has_value();
        return graphicsFamily.has_value();
    }
};
} // namespace vkStruct

class Device
{
public:
    ~Device();
    
    static vk::PhysicalDevice& PhysicalDevice();
    static vkStruct::QueueFamilyIndices& QueueFamilyIndices();
    static vk::Device& Get();

    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice& device);
    void FindQueueFamilies();

    void CreateLogicalDevice();
};

#endif