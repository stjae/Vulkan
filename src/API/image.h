#ifndef IMAGE_H
#define IMAGE_H

#include "handle.h"
#include "memory.h"

class Image
{
    ImageHandle handle_;

public:
    Memory memory;

    void CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent);
    void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateSampler();
    void SetInfo(vk::ImageLayout imageLayout);
    void DestroyImage();
    void DestroyImageView();
    ~Image();

    const ImageHandle& GetHandle() { return handle_; }
};

#endif