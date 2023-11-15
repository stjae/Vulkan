#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "API/buffer.h"
#include "image.h"

class MeshData
{
public:
    vk::VertexInputBindingDescription GetBindingDesc();
    std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescs();
    void CreateVertexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);
    void CreateIndexBuffer(const vk::PhysicalDevice& vkPhysicalDevice, const vk::Device& vkDevice);
    void CreateTexture(const Device& device, const char* fileDir);
    void DestroyStagingBuffer();

    std::vector<float> vertices;
    std::vector<uint16_t> indices;

    std::unique_ptr<Buffer> vertexStagingBuffer;
    std::unique_ptr<Buffer> vertexBuffer;

    std::unique_ptr<Buffer> indexStagingBuffer;
    std::unique_ptr<Buffer> indexBuffer;

    std::unique_ptr<Image> textureImage;
    std::unique_ptr<Buffer> textureStagingBuffer;
    int textureWidth, textureHeight;
    size_t textureSize;
};

#endif