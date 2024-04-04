#include "image.h"

vkn::Image::Image()
{
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    imageViewCreateInfo.subresourceRange = subresourceRange;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.subresourceRange = subresourceRange;
}

void vkn::Image::CreateImage(vk::Extent3D&& extent, vk::Format format, vk::ImageUsageFlags usage, vk::ImageTiling tiling, vk::MemoryPropertyFlags memoryProperty)
{
    imageCreateInfo.extent = extent;
    imageCreateInfo.format = format;
    imageCreateInfo.usage = usage;
    imageCreateInfo.tiling = tiling;

    imageBundle_.image = vkn::Device::GetBundle().device.createImage(imageCreateInfo);

    memory.AllocateMemory(imageBundle_.image, memoryProperty);
}

void vkn::Image::CreateImageView()
{
    imageViewCreateInfo.image = imageBundle_.image;
    imageViewCreateInfo.format = imageCreateInfo.format;

    imageBundle_.imageView = vkn::Device::GetBundle().device.createImageView(imageViewCreateInfo);
}

void vkn::Image::CreateSampler()
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = vk::True;
    samplerInfo.maxAnisotropy = vkn::Device::physicalDeviceLimits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    vkn::CheckResult(vkn::Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &Image::repeatSampler));

    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    // vkn::CheckResult(vkn::Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &Image::clampSampler));
}

void vkn::Image::DestroyImage()
{
    vkn::Device::GetBundle().device.destroyImage(imageBundle_.image);
    imageBundle_.image = nullptr;
}

void vkn::Image::DestroyImageView()
{
    vkn::Device::GetBundle().device.destroyImageView(imageBundle_.imageView);
    imageBundle_.imageView = nullptr;
}

vkn::Image::~Image()
{
    memory.Free();
    if (imageBundle_.image != VK_NULL_HANDLE)
        vkn::Device::GetBundle().device.destroyImage(imageBundle_.image);
    if (imageBundle_.imageView != VK_NULL_HANDLE)
        vkn::Device::GetBundle().device.destroyImageView(imageBundle_.imageView);
}
