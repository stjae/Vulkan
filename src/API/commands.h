#ifndef COMMANDS_H
#define COMMANDS_H

#include "device.h"
#include "pipeline.h"
#include "../mesh.h"

class Command
{
public:
    vk::CommandPool commandPool_;
    std::vector<vk::CommandBuffer> commandBuffers_;

    void CreateCommandPool();
    void AllocateCommandBuffer();
    void RecordDrawCommands(GraphicsPipeline& pipeline, const vk::Image& image, uint32_t imageIndex, const vk::Framebuffer& framebuffer, const vk::Extent2D& extent, std::vector<std::shared_ptr<Mesh>>& meshes, uint32_t dynamicOffset, ImDrawData* imDrawData);
    void RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    void RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height);
    void TransitImageLayout(const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void Submit();
    ~Command();
};

#endif
