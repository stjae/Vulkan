#ifndef IMAGE_H
#define IMAGE_H

#include "memory.h"

namespace vkn {
struct ImageBundle
{
    vk::Image image;
    vk::ImageView imageView;
    vk::Format format;
    // vk::Sampler sampler;
    inline static vk::Sampler globalSampler;
    vk::DescriptorImageInfo imageInfo;
};
class Image
{
    ImageBundle imageBundle_;

public:
    Memory memory;

    void CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent, vk::ImageTiling imageTiling);
    void CreateImage(vk::ImageCreateInfo& imageCI, vk::MemoryPropertyFlags memProperty);
    void CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void CreateImageView(vk::ImageViewCreateInfo& imageViewCI);
    void CreateSampler();
    static void CreateGlobalSampler();
    void SetInfo(vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined);
    void DestroyImage();
    void DestroyImageView();
    ~Image();

    const ImageBundle& GetBundle() const { return imageBundle_; }
};
} // namespace vkn

#endif