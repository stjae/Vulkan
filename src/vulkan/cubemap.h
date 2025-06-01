#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "vulkanImage.h"

namespace vkn {
class Cubemap : public Image
{
protected:
    uint32_t m_imageSize;
    std::array<vk::ImageView, 6> m_cubemapFaceImageViews;
    std::array<vk::Framebuffer, 6> m_framebuffers;

    int m_numMips;
    uint32_t m_baseMipSize;
    vk::Sampler m_mipmapSampler;

public:
    vkn::Image m_mipmap;
    vk::DescriptorImageInfo m_mipmapDescriptorImageInfo;

    Cubemap();
    void CreateCubemap(uint32_t imageSize, vk::Format format, vk::ImageUsageFlags usage, const vk::CommandBuffer& commandBuffer);
    void CreateMipmap(const vk::CommandBuffer& commandBuffer);
    void CreateMipmapSampler();
    void CopyToMipmap(uint32_t mipSize, int mipLevel, const vk::CommandBuffer& commandBuffer);
    void ChangeImageLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout);
    ~Cubemap();
};
} // namespace vkn

#endif
