#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "instance.h"
#include "queue.h"

class Device
{
    Instance instance_;
    Queue queue_;
    std::vector<const char*> deviceExtensions_;

    inline static DeviceHandle handle_;

public:
    inline static vk::PhysicalDeviceLimits limits;

    Device();
    void PickPhysicalDevice();
    bool IsDeviceSuitable(vk::PhysicalDevice vkPhysicalDevice);
    ~Device();

    static const DeviceHandle& GetHandle() { return handle_; }
};

#endif