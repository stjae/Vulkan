#ifndef IMAGE_H
#define IMAGE_H

#include "buffer.h"
#include "command.h"
#include "pipeline.h"
#include "../scene/mesh/mesh.h"

namespace vkn {
class Image
{
protected:
    struct Bundle
    {
        vk::Image image;
        vk::ImageView imageView;
        vk::DescriptorImageInfo descriptorImageInfo;
    } m_bundle;
    vk::Framebuffer m_framebuffer;
    std::unique_ptr<Buffer> m_stagingBuffer;

public:
    vk::ImageCreateInfo m_imageCreateInfo;
    vk::ImageViewCreateInfo m_imageViewCreateInfo;
    Memory m_memory;
    inline static vk::Sampler s_repeatSampler;
    inline static vk::Sampler s_clampSampler;

    Image();
    ~Image();
    const Bundle& Get() const { return m_bundle; }
    void CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty, vk::Sampler = s_repeatSampler);
    void CreateImageView();
    void CreateFramebuffer(const Pipeline& pipeline);
    void Draw(const Mesh& square, const Pipeline& pipeline, vk::CommandBuffer& commandBuffer);
    void InsertImage(const std::string& filePath, vk::Format format, vk::CommandBuffer& commandBuffer);
    void InsertDummyImage(vk::CommandBuffer& commandBuffer, std::array<uint8_t, 4>&& color = { 0, 0, 0, 255 });
    void InsertHDRImage(const std::string& filePath, vk::Format format, vk::CommandBuffer& commandBuffer);
    void DestroyImage();
    void DestroyImageView();
    static void CreateSampler();
    Image(Image&& other) { this->m_stagingBuffer = std::move(other.m_stagingBuffer); }
};
} // namespace vkn

#endif