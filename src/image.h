#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "API/memory.h"

class Image
{
public:
    Memory memory;

    Image(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice) : vkPhysicalDevice_(vkPhysicalDevice), vkDevice_(vkDevice) {}
    void CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent);
    void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void SetInfo(vk::ImageLayout imageLayout);
    vk::DescriptorImageInfo GetInfo() { return imageInfo_; };
    vk::Format GetFormat() { return format_; }
    void CreateSampler(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);
    ~Image();

private:
    vk::Image image_;
    vk::ImageView imageView_;
    vk::Format format_;
    vk::Sampler sampler_;
    vk::DescriptorImageInfo imageInfo_;

    const vk::PhysicalDevice& vkPhysicalDevice_;
    const vk::Device& vkDevice_;
};

#endif // __IMAGE_H__