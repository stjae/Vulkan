#include "mesh.h"

Mesh::Mesh()
{
    std::vector<float> pos = { -0.5f, 0.5f, 0.0f,
                               0.5f, 0.5f, 0.0f,
                               0.5f, -0.5f, 0.0f,
                               -0.5f, -0.5f, 0.0f };

    std::vector<float> color = { 1.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f,
                                 1.0f, 1.0f, 1.0f };

    vertices.reserve(pos.size() + color.size());

    for (int i = 0; i < pos.size(); i += 3) {

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(pos[j]);
        }

        for (int j = i; j < i + 3; ++j) {
            vertices.push_back(color[j]);
        }
    }

    indices = { 0, 1, 2, 2, 3, 0 };
}

vk::VertexInputBindingDescription Mesh::GetPosColorBindingDesc()
{
    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0);
    bindingDesc.setStride(3 * sizeof(float) + 3 * sizeof(float));
    bindingDesc.setInputRate(vk::VertexInputRate::eVertex);

    return bindingDesc;
}

std::array<vk::VertexInputAttributeDescription, 2> Mesh::GetPosColorAttributeDescs()
{
    std::array<vk::VertexInputAttributeDescription, 2> attributes;

    // Pos
    attributes[0].setBinding(0);
    attributes[0].setLocation(0);
    attributes[0].setFormat(vk::Format::eR32G32B32Sfloat);
    attributes[0].setOffset(0);

    // Color
    attributes[1].setBinding(0);
    attributes[1].setLocation(1);
    attributes[1].setFormat(vk::Format::eR32G32B32Sfloat);
    attributes[1].setOffset(3 * sizeof(float));

    return attributes;
}

void Mesh::CreateVertexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(vertices[0]) * vertices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    vertexStagingBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, stagingBufferInput);

    void* memoryLocation = vkDevice.mapMemory(vertexStagingBuffer->vkDeviceMemory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, vertices.data(), stagingBufferInput.size);
    vkDevice.unmapMemory(vertexStagingBuffer->vkDeviceMemory);

    BufferInput vertexBufferInput;
    vertexBufferInput.size = stagingBufferInput.size;
    vertexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    vertexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    vertexBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, vertexBufferInput);
}

void Mesh::CreateIndexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice)
{
    BufferInput stagingBufferInput;
    stagingBufferInput.size = sizeof(indices[0]) * indices.size();
    stagingBufferInput.usage = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferInput.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent;

    indexStagingBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, stagingBufferInput);

    void* memoryLocation = vkDevice.mapMemory(indexStagingBuffer->vkDeviceMemory, 0, stagingBufferInput.size);
    memcpy(memoryLocation, indices.data(), stagingBufferInput.size);
    vkDevice.unmapMemory(indexStagingBuffer->vkDeviceMemory);

    BufferInput indexBufferInput;
    indexBufferInput.size = stagingBufferInput.size;
    indexBufferInput.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    indexBufferInput.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    indexBuffer = std::make_unique<Buffer>(vkPhysicalDevice, vkDevice, indexBufferInput);
}