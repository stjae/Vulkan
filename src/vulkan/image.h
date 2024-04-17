#ifndef IMAGE_H
#define IMAGE_H

#include "buffer.h"
#include "command.h"
#include "pipeline.h"
#include "../scene/mesh/mesh.h"

namespace vkn {
struct ImageBundle
{
    vk::Image image;
    vk::ImageView imageView;
    vk::DescriptorImageInfo descriptorImageInfo;
};
class Image
{
private:
    vk::Framebuffer framebuffer_;
    std::shared_ptr<Buffer> stagingBuffer_;

protected:
    ImageBundle imageBundle_;

public:
    vk::ImageCreateInfo imageCreateInfo;
    vk::ImageViewCreateInfo imageViewCreateInfo;
    Memory memory;
    inline static vk::Sampler repeatSampler;
    inline static vk::Sampler clampSampler;

    Image();
    void CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty, vk::Sampler = repeatSampler);
    void CreateImageView();
    void CreateFramebuffer(const Pipeline& pipeline);
    void Draw(const Mesh& square, const Pipeline& pipeline, vk::CommandBuffer& commandBuffer);
    void InsertImage(const std::string& filePath, vk::Format format, vk::CommandBuffer& commandBuffer);
    void InsertDummyImage(vk::CommandBuffer& commandBuffer, std::array<uint8_t, 4>&& color = { 0, 0, 0, 255 });
    void InsertHDRImage(const std::string& filePath, vk::Format format, vk::CommandBuffer& commandBuffer);
    void DestroyImage();
    void DestroyImageView();
    static void CreateSampler();
    ~Image();

    const ImageBundle& GetBundle() const { return imageBundle_; }
};
} // namespace vkn

#endif