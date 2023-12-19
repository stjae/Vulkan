#ifndef _MESHDATA_H_
#define _MESHDATA_H_

#include "API/buffer.h"
#include "image.h"
#include <filesystem>

struct Vertices
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<uint32_t> indices;
    std::vector<int> textureIDs;
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

    Vertices vertices;

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