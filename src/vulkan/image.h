#ifndef IMAGE_H
#define IMAGE_H

#include "memory.h"

namespace vkn {
struct ImageBundle
{
    vk::Image image;
    vk::ImageView imageView;
};
class Image
{
protected:
    ImageBundle imageBundle_;

public:
    vk::ImageCreateInfo imageCreateInfo;
    vk::ImageViewCreateInfo imageViewCreateInfo;
    Memory memory;
    inline static vk::Sampler repeatSampler;
    inline static vk::Sampler clampSampler;

    Image();
    void CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty);
    void CreateImageView();
    void DestroyImage();
    void DestroyImageView();
    static void CreateSampler();
    ~Image();

    const ImageBundle& GetBundle() const { return imageBundle_; }
};
} // namespace vkn

#endif