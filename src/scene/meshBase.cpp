#include "meshBase.h"

vk::VertexInputBindingDescription MeshBase::GetBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(sizeof(Vertex));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 5> MeshBase::GetAttributeDescs()
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

    // Texcoord
    attributes[2].binding = 0;
    attributes[2].location = 2;
    attributes[2].format = vk::Format::eR32G32Sfloat;
    attributes[2].offset = offset;
    offset += sizeof(Vertex::texcoord);

    // Tangent
    attributes[3].binding = 0;
    attributes[3].location = 3;
    attributes[3].format = vk::Format::eR32G32B32Sfloat;
    attributes[3].offset = offset;
    offset += sizeof(Vertex::tangent);

    // Bitangent
    attributes[4].binding = 0;
    attributes[4].location = 4;
    attributes[4].format = vk::Format::eR32G32B32Sfloat;
    attributes[4].offset = offset;
    offset += sizeof(Vertex::bitangent);

    return attributes;
}

void MeshBase::CreateVertexBuffers(std::vector<Vertex>& vertices)
{
    m_vertexStagingBuffers.emplace_back();
    m_vertexBuffers.emplace_back();

    vkn::BufferInfo bufferInput = { sizeof(Vertex) * vertices.size(), sizeof(Vertex), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_vertexStagingBuffers.back() = std::make_unique<vkn::Buffer>(bufferInput);
    m_vertexStagingBuffers.back()->Copy(vertices.data());

    bufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    m_vertexBuffers.back() = std::make_unique<vkn::Buffer>(bufferInput);
}

void MeshBase::CreateIndexBuffers(std::vector<uint32_t>& indices)
{
    m_indexStagingBuffers.emplace_back();
    m_indexBuffers.emplace_back();

    vkn::BufferInfo bufferInput = { sizeof(uint32_t) * indices.size(), sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
    m_indexStagingBuffers.back() = std::make_unique<vkn::Buffer>(bufferInput);
    m_indexStagingBuffers.back()->Copy(indices.data());

    bufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    bufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    m_indexBuffers.back() = std::make_unique<vkn::Buffer>(bufferInput);
}
