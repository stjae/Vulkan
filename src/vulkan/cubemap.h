#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "image.h"

namespace vkn {
class Cubemap : public Image
{
protected:
    uint32_t m_imageSize;
    std::array<vk::ImageView, 6> m_cubemapFaceImageViews;
    std::array<vk::Framebuffer, 6> m_framebuffers;

public:
    Cubemap();
    void CreateCubemap(uint32_t imageSize, vk::Format format, vk::ImageUsageFlags usage, vk::CommandBuffer& commandBuffer);
    ~Cubemap();
};
} // namespace vkn

#endif
