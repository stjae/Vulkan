#include "meshData.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

vk::VertexInputBindingDescription MeshData::GetBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(3 * sizeof(float) + 3 * sizeof(float) + 2 * sizeof(float));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 3> MeshData::GetAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 3> attributes;

    // Pos
    attributes[0].setBinding(0);
    attributes[0].setLocation(0);
    attributes[0].setFormat(vk::Format::eR32G32B32Sfloat);
    attributes[0].setOffset(0);

    // Normal
    attributes[1].setBinding(0);
    attributes[1].setLocation(1);
    attributes[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributes[1].setOffset(3 * sizeof(float));

    // Texcoord
    attributes[2].setBinding(0);
    attributes[2].setLocation(2);
    attributes[2].setFormat(vk::Format::eR32G32Sfloat);
    attributes[2].setOffset(3 * sizeof(float) + 3 * sizeof(float));

    return attributes;
}

void MeshData::CreateVertexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput stagingBufferInput = { sizeof(vertices[0]) * vertices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    vertexStagingBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, stagingBufferInput);
    vertexStagingBuffer->CopyResource(vertices.data(), stagingBufferInput);

    BufferInput vertexBufferInput = { stagingBufferInput.size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    vertexBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, vertexBufferInput);
}

void MeshData::CreateIndexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput stagingBufferInput = { sizeof(indices[0]) * indices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    indexStagingBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, stagingBufferInput);
    indexStagingBuffer->CopyResource(indices.data(), stagingBufferInput);

    BufferInput indexBufferInput = { stagingBufferInput.size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    indexBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, indexBufferInput);
}

void MeshData::CreateTexture(const Device& device)
{
    if (textureFilePath) {

        int width, height, channel;
        vk::DeviceSize imageSize;
        stbi_uc* imageData;

        std::string filePath(PROJECT_DIR);
        filePath.append(textureFilePath);

        imageData = stbi_load(filePath.c_str(), &width, &height, &channel, STBI_rgb_alpha);
        imageSize = width * height * 4;

        if (!imageData) {
            spdlog::error("failed to load texture");
            return;
        }

        textureWidth = width;
        textureHeight = height;
        textureSize = static_cast<size_t>(imageSize);

        BufferInput input = { imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        textureStagingBuffer = std::make_unique<Buffer>(device.vkPhysicalDevice, device.vkDevice, input);
        textureStagingBuffer->CopyResource(imageData, input);

        stbi_image_free(imageData);
        textureImage = std::make_unique<Image>(device.vkPhysicalDevice, device.vkDevice);
        vk::Extent3D extent(width, height, 1);
        textureImage->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        textureImage->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

    } else {

        // Create dummy texture
        unsigned char dummyTexture[4] = { 255, 255, 255, 255 };
        textureWidth = 1;
        textureHeight = 1;
        textureSize = 4;

        BufferInput input = { 4, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        textureStagingBuffer = std::make_unique<Buffer>(device.vkPhysicalDevice, device.vkDevice, input);
        textureStagingBuffer->CopyResource(&dummyTexture, input);

        textureImage = std::make_unique<Image>(device.vkPhysicalDevice, device.vkDevice);
        vk::Extent3D extent(1, 1, 1);
        textureImage->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        textureImage->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    }
}

void MeshData::DestroyStagingBuffer()
{
    indexStagingBuffer->DestroyBuffer();
    vertexStagingBuffer->DestroyBuffer();
    textureStagingBuffer->DestroyBuffer();
}
