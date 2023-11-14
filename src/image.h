#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "API/memory.h"

class Image
{
public:
    vk::Image image;
    vk::ImageView imageView;
    vk::Format format;

    Memory memory;

    Image(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice) : vkPhysicalDevice(vkPhysicalDevice), vkDevice(vkDevice) {}
    void CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent);
    void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void TransitImageLayout(vk::CommandBuffer& commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    ~Image();

    const vk::PhysicalDevice& vkPhysicalDevice;
    const vk::Device& vkDevice;
};

#endif // __IMAGE_H__