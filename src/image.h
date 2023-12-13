#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "API/memory.h"

class Image
{
public:
    Memory memory;

    void CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent);
    void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateSampler();
    void SetInfo(vk::ImageLayout imageLayout);
    void DestroyImage();
    void DestroyImageView();
    ~Image();

    vk::Image GetImage() { return image_; }
    vk::ImageView GetImageView() { return imageView_; }
    vk::Format GetFormat() { return format_; }
    vk::DescriptorImageInfo& GetInfo() { return imageInfo_; };

private:
    vk::Image image_;
    vk::ImageView imageView_;
    vk::Format format_;
    vk::Sampler sampler_;
    vk::DescriptorImageInfo imageInfo_;
};

#endif // __IMAGE_H__