#ifndef IMAGE_H
#define IMAGE_H

#include "buffer.h"
#include "command.h"

namespace vkn {
struct ImageBundle
{
    vk::Image image;
    vk::ImageView imageView;
    vk::DescriptorImageInfo descriptorImageInfo;
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
    void CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty, vk::Sampler = vkn::Image::repeatSampler);
    void CreateImageView();
    void InsertImage(const std::string& filePath, vk::CommandBuffer& commandBuffer);
    void InsertImageArrays(const std::array<std::string, 2>& filePaths, vk::CommandBuffer& commandBuffer);
    void DestroyImage();
    void DestroyImageView();
    static void CreateSampler();
    ~Image();

    const ImageBundle& GetBundle() const { return imageBundle_; }
};
} // namespace vkn

#endif