#ifndef GRID_H
#define GRID_H

#include <vector>
#include "line.h"
#include "../vulkan/buffer.h"
#include "../vulkan/command.h"

class Grid
{
    friend class Viewport;

    std::vector<LinePoint> m_linePoints;
    std::vector<uint32_t> m_lineIndices;

    std::unique_ptr<vkn::Buffer> m_vertexStagingBuffer;
    std::unique_ptr<vkn::Buffer> m_vertexBuffer;
    std::unique_ptr<vkn::Buffer> m_indexStagingBuffer;
    std::unique_ptr<vkn::Buffer> m_indexBuffer;

public:
    void CreateGrid(int gridWidth);
    void CreateBuffer();
    void CopyBuffer(const vk::CommandBuffer& commandBuffer) const;
};

#endif
