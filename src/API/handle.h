#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <vulkan/vulkan.hpp>

struct DeviceHandle
{
    vk::Device device;
    vk::PhysicalDevice physicalDevice;
};

struct QueueHandle
{
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
};

struct InstanceHandle
{
    VkSurfaceKHR surface;
    vk::Instance instance;
};

struct PipelineHandle
{
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
};

#endif // _HANDLE_H_