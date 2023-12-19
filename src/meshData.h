#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "API/buffer.h"
#include "image.h"
#include <filesystem>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texcoord;
    int textureID;
};

class MeshData
{
public:
    static vk::VertexInputBindingDescription GetBindingDesc();
    static std::array<vk::VertexInputAttributeDescription, 4> GetAttributeDescs();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateTexture();
    void DestroyStagingBuffer();

    std::vector<Vertex> vertices;
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