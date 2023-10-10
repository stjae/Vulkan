#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "config.h"
#include "instance.h"

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
};

#endif