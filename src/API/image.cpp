#include "image.h"

void Image::CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent, vk::ImageTiling imageTiling)
{
    vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, imageTiling, usage);

    imageBundle_.image = Device::GetBundle().device.createImage(createInfo);
    imageBundle_.format = format;
    memory.AllocateMemory(imageBundle_.image, properties);
}

void Image::CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageSubresourceRange range(aspectFlags, 0, 1, 0, 1);
    vk::ImageViewCreateInfo createInfo({}, imageBundle_.image, vk::ImageViewType::e2D, format, {}, range);

    imageBundle_.imageView = Device::GetBundle().device.createImageView(createInfo);
}

void Image::CreateSampler()
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = vk::True;
    samplerInfo.maxAnisotropy = Device::physicalDeviceLimits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = vk::False;
    samplerInfo.compareEnable = vk::False;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &imageBundle_.sampler) != vk::Result::eSuccess) {
        spdlog::error("failed to create sampler");
    }
}

void Image::SetInfo(vk::ImageLayout imageLayout)
{
    vk::DescriptorImageInfo imageInfo(imageBundle_.sampler, imageBundle_.imageView, imageLayout);
    imageBundle_.imageInfo = imageInfo;
}

void Image::DestroyImage()
{
    Device::GetBundle().device.destroyImage(imageBundle_.image);
    imageBundle_.image = VK_NULL_HANDLE;
}

void Image::DestroyImageView()
{
    Device::GetBundle().device.destroyImageView(imageBundle_.imageView);
    imageBundle_.imageView = VK_NULL_HANDLE;
}

Image::~Image()
{
    memory.Free();
    if (imageBundle_.image != VK_NULL_HANDLE)
        Device::GetBundle().device.destroyImage(imageBundle_.image);
    if (imageBundle_.imageView != VK_NULL_HANDLE)
        Device::GetBundle().device.destroyImageView(imageBundle_.imageView);
    if (imageBundle_.sampler != VK_NULL_HANDLE)
        Device::GetBundle().device.destroySampler(imageBundle_.sampler);
}