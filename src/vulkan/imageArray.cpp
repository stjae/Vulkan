#include "imageArray.h"

namespace vkn {
ImageArray::ImageArray(uint32_t layerCount)
{
}

void ImageArray::InsertImageArrays(const std::vector<std::string>& filePaths, vk::CommandBuffer& commandBuffer)
{
    // Get texture info from first texture
    int width = 0, height = 0, channel = 0;
    vk::DeviceSize imageSize = 0;
    stbi_uc* imageData = stbi_load(filePaths[0].c_str(), &width, &height, &channel, STBI_rgb_alpha);
    stbi_image_free(imageData);

    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.arrayLayers = 4;
    CreateImage({ (uint32_t)width, (uint32_t)height, 1 }, vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageTiling::eOptimal, vk::MemoryPropertyFlagBits::eDeviceLocal);
    imageViewCreateInfo.viewType = vk::ImageViewType::e2DArray;

    for (uint32_t i = 0; i < 4; i++) {
        imageData = stbi_load(filePaths[i].c_str(), &width, &height, &channel, STBI_rgb_alpha);

        if (imageData) {
            spdlog::info("load texture from [{}]", filePaths[i].c_str());
            imageSize = width * height * 4;
        } else {
            spdlog::error("failed to load texture");
        }

        BufferInput bufferInput = { imageSize, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        Buffer stagingBuffer(bufferInput);
        if (imageData)
            stagingBuffer.Copy(imageData);

        stbi_image_free(imageData);

        imageViewCreateInfo.subresourceRange.baseArrayLayer = i;
        CreateImageView();

        Command::Begin(commandBuffer);
        // Set texture image layout to transfer dst optimal
        Command::SetImageMemoryBarrier(commandBuffer,
                                       imageBundle_.image,
                                       imageBundle_.descriptorImageInfo,
                                       vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eTransferDstOptimal,
                                       {},
                                       vk::AccessFlagBits::eTransferWrite,
                                       vk::PipelineStageFlagBits::eTopOfPipe,
                                       vk::PipelineStageFlagBits::eTransfer,
                                       { vk::ImageAspectFlagBits::eColor, 0, 1, i, 1 });
        // Copy texture image from staging buffer
        vkn::Command::CopyBufferToImage(commandBuffer,
                                        stagingBuffer.GetBundle().buffer,
                                        imageBundle_.image, width,
                                        height,
                                        i);
        // Set texture image layout to shader read only
        vkn::Command::SetImageMemoryBarrier(commandBuffer,
                                            imageBundle_.image,
                                            imageBundle_.descriptorImageInfo,
                                            vk::ImageLayout::eTransferDstOptimal,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            vk::AccessFlagBits::eTransferWrite,
                                            vk::AccessFlagBits::eShaderRead,
                                            vk::PipelineStageFlagBits::eTransfer,
                                            vk::PipelineStageFlagBits::eFragmentShader,
                                            { vk::ImageAspectFlagBits::eColor, 0, 1, i, 1 });
        commandBuffer.end();
        vkn::Command::Submit(&commandBuffer, 1);
    }
}
} // namespace vkn