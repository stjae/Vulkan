#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "API/buffer.h"
#include "image.h"
#include <filesystem>

class MeshData
{
public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescs();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateTexture();
    void DestroyStagingBuffer();

    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    std::unique_ptr<Buffer> vertexStagingBuffer;
    std::unique_ptr<Buffer> vertexBuffer;

    std::unique_ptr<Buffer> indexStagingBuffer;
    std::unique_ptr<Buffer> indexBuffer;

    std::unique_ptr<Image> textureImage;
    std::unique_ptr<Buffer> textureStagingBuffer;
    int textureWidth, textureHeight;
    size_t textureSize;
    const char* textureFilePath;
};

#endif