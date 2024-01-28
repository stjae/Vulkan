#include "meshData.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

vk::VertexInputBindingDescription MeshData::GetBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(3 * sizeof(float) + 3 * sizeof(float) + 2 * sizeof(float) + 1 * sizeof(int));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 4> MeshData::GetAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 4> attributes;

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

    // TextureID
    attributes[3].setBinding(0);
    attributes[3].setLocation(3);
    attributes[3].setFormat(vk::Format::eR32Sint);
    attributes[3].setOffset(3 * sizeof(float) + 3 * sizeof(float) + 2 * sizeof(float));

    return attributes;
}

void MeshData::CreateVertexBuffer()
{
    BufferInput stagingBufferInput = { sizeof(Vertex) * vertices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    vertexStagingBuffer = std::make_unique<Buffer>(stagingBufferInput);
    vertexStagingBuffer->CopyToBuffer(vertices.data(), stagingBufferInput);

    BufferInput vertexBufferInput = { stagingBufferInput.size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    vertexBuffer = std::make_unique<Buffer>(vertexBufferInput);
}

void MeshData::CreateIndexBuffer()
{
    BufferInput stagingBufferInput = { sizeof(uint32_t) * indices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    indexStagingBuffer = std::make_unique<Buffer>(stagingBufferInput);
    indexStagingBuffer->CopyToBuffer(indices.data(), stagingBufferInput);

    BufferInput indexBufferInput = { stagingBufferInput.size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    indexBuffer = std::make_unique<Buffer>(indexBufferInput);
}

void MeshData::CreateTexture()
{
    if (textureFilePath) {

        int width, height, channel;
        vk::DeviceSize imageSize;
        stbi_uc* imageData;

        imageData = stbi_load(textureFilePath, &width, &height, &channel, STBI_rgb_alpha);
        imageSize = width * height * 4;

        if (!imageData) {
            spdlog::error("failed to load texture");
            return;
        }

        textureWidth = width;
        textureHeight = height;
        textureSize = static_cast<size_t>(imageSize);

        BufferInput input = { imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        textureStagingBuffer = std::make_unique<Buffer>(input);
        textureStagingBuffer->CopyToBuffer(imageData, input);

        stbi_image_free(imageData);
        vk::Extent3D extent(width, height, 1);
        textureImage = std::make_unique<Image>();
        textureImage->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        textureImage->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
        textureImage->SetInfo();

    } else {

        // Create dummy texture
        unsigned char dummyTexture[4] = { 128, 128, 128, 128 };
        textureWidth = 1;
        textureHeight = 1;
        textureSize = 4;

        BufferInput input = { 4, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        textureStagingBuffer = std::make_unique<Buffer>(input);
        textureStagingBuffer->CopyToBuffer(&dummyTexture, input);

        vk::Extent3D extent(1, 1, 1);
        textureImage = std::make_unique<Image>();
        textureImage->CreateImage(vk::Format::eR8G8B8A8Srgb, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, extent);
        textureImage->CreateImageView(vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    }
}

void MeshData::DestroyStagingBuffer()
{
    indexStagingBuffer->Destroy();
    vertexStagingBuffer->Destroy();
    textureStagingBuffer->Destroy();
}
