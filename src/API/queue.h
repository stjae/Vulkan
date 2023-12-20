#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "../common.h"

class Queue
{
    inline static std::optional<uint32_t> graphicsFamily_;
    inline static std::optional<uint32_t> presentFamily_;
    inline static QueueHandle handle_;

public:
    void FindQueueFamilies(const vk::PhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);
    void SetDeviceQueueCreateInfo(std::vector<vk::DeviceQueueCreateInfo>& deviceQueueCreateInfos);

    static const QueueHandle& GetHandle() { return handle_; }
    void SetHandle(vk::Queue graphicsQueueHandle, vk::Queue presentQueueHandle)
    {
        handle_.graphicsQueue = graphicsQueueHandle;
        handle_.presentQueue = presentQueueHandle;
    }
    static uint32_t GetGraphicsQueueFamilyIndex() { return graphicsFamily_.value(); }
    static uint32_t GetPresentQueueFamilyIndex() { return presentFamily_.value(); }
};

#endif // _QUEUE_H_