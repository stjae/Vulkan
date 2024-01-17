#ifndef COMMANDS_H
#define COMMANDS_H

#include "device.h"
#include "pipeline.h"
#include "image.h"
#include "../mesh.h"

class Command
{
public:
    vk::CommandPool commandPool_;
    std::vector<vk::CommandBuffer> commandBuffers_;

    void CreateCommandPool();
    void AllocateCommandBuffer();
    void RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    void RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height);
    void TransitImageLayout(Image* image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags);
    void Submit();
    ~Command();
};

#endif
