#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "API/buffer.h"
#include "API/image.h"

class MeshData
{
protected:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 4> GetAttributeDescs();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateTexture();
    void DestroyStagingBuffer();

    size_t GetVertexCount() { return vertices.size(); }
    size_t GetIndexCount() { return indices.size(); }

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