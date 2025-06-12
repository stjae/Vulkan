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
    CreateImageView();

    m_imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    m_imageViewCreateInfo.subresourceRange.layerCount = 1;
    for (uint32_t i = 0; i < 6; i++) {
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::CHECK_RESULT(vkn::Device::Get().device.createImageView(&m_imageViewCreateInfo, nullptr, &m_cubemapFaceImageViews[i]));
    }
}

void Cubemap::CreateMipmap(const vk::CommandBuffer& commandBuffer)
{
    m_mipmap.m_imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    m_mipmap.m_imageCreateInfo.imageType = vk::ImageType::e2D;
    m_mipmap.m_imageCreateInfo.arrayLayers = 6;
    m_mipmap.CreateImage({ m_baseMipSize, m_baseMipSize, 1 }, vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal, s_repeatSampler, m_numMips);

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, m_numMips, 0, 6);
    m_mipmap.m_imageViewCreateInfo.subresourceRange = subresourceRange;
    m_mipmap.m_imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
    m_mipmap.CreateImageView();

    CreateMipmapSampler();
}

void Cubemap::CopyToMipmap(uint32_t mipSize, int mipLevel, const vk::CommandBuffer& commandBuffer)
{
    for (int i = 0; i < 6; i++) {
        vk::ImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        copyRegion.srcSubresource.baseArrayLayer = i;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;

        copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        copyRegion.dstSubresource.baseArrayLayer = i;
        copyRegion.dstSubresource.mipLevel = mipLevel;
        copyRegion.dstSubresource.layerCount = 1;

        copyRegion.extent.width = mipSize;
        copyRegion.extent.height = mipSize;
        copyRegion.extent.depth = 1;

        commandBuffer.copyImage(m_bundle.image,
                                vk::ImageLayout::eTransferSrcOptimal,
                                m_mipmap.Get().image,
                                vk::ImageLayout::eTransferDstOptimal,
                                1,
                                &copyRegion);
    }
}

void Cubemap::CreateMipmapSampler()
{
    vk::SamplerCreateInfo samplerCI;
    samplerCI.magFilter = vk::Filter::eLinear;
    samplerCI.minFilter = vk::Filter::eLinear;
    samplerCI.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerCI.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerCI.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerCI.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    samplerCI.minLod = 0.0f;
    samplerCI.maxLod = static_cast<float>(m_numMips);
    samplerCI.borderColor = vk::BorderColor::eFloatOpaqueWhite;

    vkn::CHECK_RESULT(vkn::Device::Get().device.createSampler(&samplerCI, nullptr, &m_mipmapSampler));
}

void Cubemap::ChangeImageLayout(const vk::CommandBuffer& commandBuffer, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout)
{
    for (int i = 0; i < 6; i++) {
        m_imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        vkn::Command::ChangeImageLayout(commandBuffer, m_bundle.image, oldImageLayout, newImageLayout, m_imageViewCreateInfo.subresourceRange);
    }
    m_bundle.descriptorImageInfo.imageLayout = newImageLayout;
}

Cubemap::~Cubemap()
{
    vkn::Device::Get().device.destroySampler(m_mipmapSampler);
    for (auto& framebuffer : m_framebuffers)
        vkn::Device::Get().device.destroyFramebuffer(framebuffer);
    for (auto& imageView : m_cubemapFaceImageViews)
        vkn::Device::Get().device.destroyImageView(imageView);
}

} // namespace vkn