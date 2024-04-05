#ifndef IMAGEARRAY_H
#define IMAGEARRAY_H

#include "image.h"

namespace vkn {

class ImageArray : public Image
{
    explicit ImageArray(uint32_t layerCount);
    void InsertImageArrays(const std::vector<std::string>& filePaths, vk::CommandBuffer& commandBuffer);
};

} // namespace vkn

#endif
