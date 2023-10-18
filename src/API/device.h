#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "config.h"
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

#endif