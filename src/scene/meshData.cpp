#include "meshData.h"

vk::VertexInputBindingDescription MeshData::GetBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(sizeof(Vertex));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 4> MeshData::GetAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 4> attributes;
    uint32_t offset = 0;

    // Pos
    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = vk::Format::eR32G32B32Sfloat;
    attributes[0].offset = offset;
    offset += sizeof(Vertex::pos);

    // Normal
    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = vk::Format::eR32G32B32Sfloat;
    attributes[1].offset = offset;
    offset += sizeof(Vertex::normal);

    // Normal
    attributes[2].binding = 0;
    attributes[2].location = 2;
    attributes[2].format = vk::Format::eR32G32B32Sfloat;
    attributes[2].offset = offset;
    offset += sizeof(Vertex::color);

    // Texcoord
    attributes[3].binding = 0;
    attributes[3].location = 3;
    attributes[3].format = vk::Format::eR32G32Sfloat;
    attributes[3].offset = offset;
    offset += sizeof(Vertex::texcoord);

    return attributes;
}

void MeshData::CreateVertexBuffer()
{
    BufferInput stagingBufferInput = { sizeof(Vertex) * vertices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    vertexStagingBuffer = std::make_unique<Buffer>(stagingBufferInput);
    vertexStagingBuffer->CopyResourceToBuffer(vertices.data(), stagingBufferInput);

    BufferInput vertexBufferInput = { stagingBufferInput.size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    vertexBuffer = std::make_unique<Buffer>(vertexBufferInput);

    // store x2 of vertex count for drawing normal
    BufferInput vertexStorageBufferInput = { stagingBufferInput.size * 2, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    vertexStorageBuffer = std::make_unique<Buffer>(vertexStorageBufferInput);
}

void MeshData::CreateIndexBuffer()
{
    BufferInput stagingBufferInput = { sizeof(uint32_t) * indices.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    indexStagingBuffer = std::make_unique<Buffer>(stagingBufferInput);
    indexStagingBuffer->CopyResourceToBuffer(indices.data(), stagingBufferInput);

    BufferInput indexBufferInput = { stagingBufferInput.size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal };
    indexBuffer = std::make_unique<Buffer>(indexBufferInput);
}
