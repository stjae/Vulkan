#ifndef HANDLE_H
#define HANDLE_H

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
    VkSurfaceKHR surface{};
    vk::Instance instance;
};

struct PipelineHandle
{
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    vk::RenderPass renderPass;
};

struct BufferHandle
{
    vk::Buffer buffer;
    vk::DescriptorBufferInfo bufferInfo;
    vk::DeviceMemory bufferMemory;
};

struct ImageHandle
{
    vk::Image image;
    vk::ImageView imageView;
    vk::Format format;
    vk::Sampler sampler;
    vk::DescriptorImageInfo imageInfo;
};

#endif