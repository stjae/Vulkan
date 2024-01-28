#ifndef IMAGE_H
#define IMAGE_H

#include "memory.h"

struct ImageBundle
{
    vk::Image image;
    vk::ImageView imageView;
    vk::Format format;
    vk::Sampler sampler;
    vk::DescriptorImageInfo imageInfo;
};

class Image
{
    ImageBundle imageBundle_;

public:
    Memory memory;

    void CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent);
    void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateSampler();
    void SetInfo(vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined);
    void DestroyImage();
    void DestroyImageView();
    ~Image();

    const ImageBundle& GetBundle() const { return imageBundle_; }
};

#endif