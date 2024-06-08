#include "cubemap.h"

namespace vkn {

Cubemap::Cubemap()
{
    m_imageSize = 0;
    m_imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    m_imageCreateInfo.imageType = vk::ImageType::e2D;
    m_imageCreateInfo.mipLevels = 1;
    m_imageCreateInfo.arrayLayers = 6;

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
    m_imageViewCreateInfo.subresourceRange = subresourceRange;
    m_imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
}

void Cubemap::CreateCubemap(uint32_t imageSize, vk::Format format, vk::ImageUsageFlags usage, const vk::CommandBuffer& commandBuffer)
{
    m_imageSize = imageSize;
    CreateImage({ imageSize, imageSize, 1 }, format, usage, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);

    vkn::Command::Begin(commandBuffer);
    vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                        m_bundle.image,
                                        vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eShaderReadOnlyOptimal,
                                        vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eHostWrite,
                                        vk::AccessFlagBits::eShaderRead,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        vk::PipelineStageFlagBits::eAllCommands,
                                        m_imageViewCreateInfo.subresourceRange);
    commandBuffer.end();
    vkn::Command::SubmitAndWait(commandBuffer);

    CreateImageView();

    m_imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    m_imageViewCreateInfo.subresourceRange.layerCount = 1;
    for (uint32_t i = 0; i < 6; i++) {
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::CheckResult(vkn::Device::Get().device.createImageView(&m_imageViewCreateInfo, nullptr, &m_cubemapFaceImageViews[i]));
    }
}

Cubemap::~Cubemap()
{
    for (auto& framebuffer : m_framebuffers)
        vkn::Device::Get().device.destroyFramebuffer(framebuffer);
    for (auto& imageView : m_cubemapFaceImageViews)
        vkn::Device::Get().device.destroyImageView(imageView);
}

} // namespace vkn