#include "cubemap.h"

namespace vkn {

Cubemap::Cubemap()
{
    imageSize_ = 0;
    imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 6;

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
    imageViewCreateInfo.subresourceRange = subresourceRange;
    imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
}

void Cubemap::CreateCubemap(uint32_t imageSize, vk::Format format, vk::ImageUsageFlags usage, vk::CommandBuffer& commandBuffer)
{
    imageSize_ = imageSize;
    CreateImage({ imageSize, imageSize, 1 }, format, usage, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        imageBundle_.image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    vkn::Command::Submit(&commandBuffer, 1);

    CreateImageView();

    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    for (uint32_t i = 0; i < 6; i++) {
        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::CheckResult(vkn::Device::GetBundle().device.createImageView(&imageViewCreateInfo, nullptr, &cubemapFaceImageViews_[i]));
    }
}

Cubemap::~Cubemap()
{
    for (auto& framebuffer : framebuffers_)
        vkn::Device::GetBundle().device.destroyFramebuffer(framebuffer);
    for (auto& imageView : cubemapFaceImageViews_)
        vkn::Device::GetBundle().device.destroyImageView(imageView);
}

} // namespace vkn