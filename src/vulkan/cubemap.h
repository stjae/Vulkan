#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "image.h"

namespace vkn {
class Cubemap : public Image
{
protected:
    std::array<vk::ImageView, 6> cubemapFaceImageViews_;
    std::array<vk::Framebuffer, 6> framebuffers_;

    uint32_t imageSize_;

public:
    Cubemap();
    void CreateCubemap(uint32_t imageSize, vk::Format format, vk::ImageUsageFlags usage, vk::CommandBuffer& commandBuffer);
    ~Cubemap();
};
} // namespace vkn

#endif
