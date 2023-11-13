#include "meshData.h"

vk::VertexInputBindingDescription MeshData::GetBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(3 * sizeof(float) + 3 * sizeof(float));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 2> MeshData::GetAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 2> attributes;

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

    return attributes;
}

void MeshData::CreateVertexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(vertices[0]) * vertices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    vertexStagingBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice);
    vertexStagingBuffer->CreateBuffer(stagingBufferInput);

    void* memoryLocation = vkDevice.mapMemory(vertexStagingBuffer->memory.vkDeviceMemory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, vertices.data(), stagingBufferInput.size);
    vkDevice.unmapMemory(vertexStagingBuffer->memory.vkDeviceMemory);

    BufferInput vertexBufferInput;
    vertexBufferInput.size = stagingBufferInput.size;
    vertexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    vertexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    vertexBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice);
    vertexBuffer->CreateBuffer(vertexBufferInput);
}

void MeshData::CreateIndexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(indices[0]) * indices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    indexStagingBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice);
    indexStagingBuffer->CreateBuffer(stagingBufferInput);

    void* memoryLocation = vkDevice.mapMemory(indexStagingBuffer->memory.vkDeviceMemory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, indices.data(), stagingBufferInput.size);
    vkDevice.unmapMemory(indexStagingBuffer->memory.vkDeviceMemory);

    BufferInput indexBufferInput;
    indexBufferInput.size = stagingBufferInput.size;
    indexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    indexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    indexBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice);
    indexBuffer->CreateBuffer(indexBufferInput);
}