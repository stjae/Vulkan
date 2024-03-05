#include "meshData.h"

vk::VertexInputBindingDescription MeshData::GetBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(sizeof(Vertex));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 5> MeshData::GetAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 5> attributes;
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

    // Tangent
    attributes[4].binding = 0;
    attributes[4].location = 4;
    attributes[4].format = vk::Format::eR32G32B32Sfloat;
    attributes[4].offset = offset;
    offset += sizeof(Vertex::tangent);

    return attributes;
}

void MeshData::CreateVertexBuffer()
{
    BufferInput bufferInput = { sizeof(Vertex) * vertices.size(), sizeof(Vertex), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    vertexStagingBuffer = std::make_unique<Buffer>(bufferInput);
    vertexStagingBuffer->Copy(vertices.data());

    bufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vertexBuffer = std::make_unique<Buffer>(bufferInput);
}

void MeshData::CreateIndexBuffer()
{
    BufferInput bufferInput = { sizeof(uint32_t) * indices.size(), sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    indexStagingBuffer = std::make_unique<Buffer>(bufferInput);
    indexStagingBuffer->Copy(indices.data());

    bufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    bufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    indexBuffer = std::make_unique<Buffer>(bufferInput);
}
