#include "cubeMap.h"

vkn::CubeMap::CubeMap()
{
    imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 6;

    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);
    imageViewCreateInfo.subresourceRange = subresourceRange;
    imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
    imageViewCreateInfo.subresourceRange = subresourceRange;
}
