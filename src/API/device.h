#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "../common.h"
#include "instance.h"
#include "struct.h"

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

    // SwapChain related method
    void QuerySwapchainSupportDetails();
    void CreateSwapchain();
    vk::SurfaceFormatKHR ChooseSurfaceFormat();
    vk::PresentModeKHR ChoosePresentMode();
    vk::Extent2D ChooseExtent();

    // Vulkan object variable
    static vk::PhysicalDevice physicalDevice;
    static std::vector<const char*> extensions;
    static QueueFamilyIndices queueFamilyIndices;
    static vk::Device device;
    static vk::Queue graphicsQueue;
    static vk::Queue presentQueue;
    static SwapchainSupportDetails swapchainSupportDetails;
    static SwapchainDetails swapchainDetails;
};

#endif