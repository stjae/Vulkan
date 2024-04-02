#include "image.h"
#include "device.h"

void vkn::Image::CreateImage(vk::Format format, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Extent3D extent, vk::ImageTiling imageTiling)
{
    vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, format, extent, 1, 1, vk::SampleCountFlagBits::e1, imageTiling, usage);

    imageBundle_.image = vkn::Device::GetBundle().device.createImage(createInfo);
    imageBundle_.format = format;
    memory.AllocateMemory(imageBundle_.image, properties);
}

void vkn::Image::CreateImage(vk::ImageCreateInfo& imageCI, vk::MemoryPropertyFlags memProperty)
{
    imageBundle_.image = vkn::Device::GetBundle().device.createImage(imageCI);
    imageBundle_.format = imageCI.format;
    memory.AllocateMemory(imageBundle_.image, memProperty);
}

void vkn::Image::CreateImageView(vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    vk::ImageSubresourceRange range(aspectFlags, 0, 1, 0, 1);
    vk::ImageViewCreateInfo createInfo({}, imageBundle_.image, vk::ImageViewType::e2D, format, {}, range);

    imageBundle_.imageView = vkn::Device::GetBundle().device.createImageView(createInfo);
}

void vkn::Image::CreateImageView(vk::ImageViewCreateInfo& imageViewCI)
{
    imageBundle_.imageView = vkn::Device::GetBundle().device.createImageView(imageViewCI);
}

void vkn::Image::CreateSampler()
{
    // vk::SamplerCreateInfo samplerInfo;
    // samplerInfo.magFilter = vk::Filter::eLinear;
    // samplerInfo.minFilter = vk::Filter::eLinear;
    // samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    // samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    // samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    // samplerInfo.anisotropyEnable = vk::True;
    // samplerInfo.maxAnisotropy = Device::physicalDeviceLimits.maxSamplerAnisotropy;
    // samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    // samplerInfo.unnormalizedCoordinates = vk::False;
    // samplerInfo.compareEnable = vk::False;
    // samplerInfo.compareOp = vk::CompareOp::eAlways;
    // samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    // samplerInfo.mipLodBias = 0.0f;
    // samplerInfo.minLod = 0.0f;
    // samplerInfo.maxLod = 0.0f;
    //
    // if (Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &imageBundle_.sampler) != vk::Result::eSuccess) {
    //     spdlog::error("failed to create sampler");
    // }
}

void vkn::Image::CreateGlobalSampler()
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

    if (vkn::Device::GetBundle().device.createSampler(&samplerInfo, nullptr, &vkn::ImageBundle::globalSampler) != vk::Result::eSuccess) {
        spdlog::error("failed to create global sampler");
    }
}

void vkn::Image::SetInfo(vk::ImageLayout imageLayout)
{
    vk::DescriptorImageInfo imageInfo(vkn::ImageBundle::globalSampler, imageBundle_.imageView, imageLayout);
    imageBundle_.imageInfo = imageInfo;
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
    // if (imageBundle_.sampler != VK_NULL_HANDLE)
    //     Device::GetBundle().device.destroySampler(imageBundle_.sampler);
}
