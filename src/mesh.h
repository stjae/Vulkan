#ifndef _MESH_H_
#define _MESH_H_

#include "common.h"
#include "API/buffer.h"

class Mesh
{
public:
    Mesh();
    vk::VertexInputBindingDescription GetPosColorBindingDesc();
    std::array<vk::VertexInputAttributeDescription, 2> GetPosColorAttributeDescs();
    void CreateVertexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);
    void CreateIndexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);

    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    std::unique_ptr<Buffer> vertexStagingBuffer;
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> indexStagingBuffer;
    std::unique_ptr<Buffer> indexBuffer;
};

#endif