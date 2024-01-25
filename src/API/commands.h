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
    const char* user_;

    void CreateCommandPool(const char* user = nullptr);

    void AllocateCommandBuffer(vk::CommandBuffer& commandBuffer);
    static void RecordCopyCommands(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    static void RecordCopyCommands(vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Image& dstImage, int width, int height);
    static void Transit(vk::CommandBuffer& commandBuffer, Image* image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags);
    static void Submit(vk::CommandBuffer* commandBuffers, uint32_t count);

    void AllocateCommandBuffer();
    void TransitImageLayout(Image* image, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::AccessFlags srcAccessFlags, vk::AccessFlags dstAccessFlags, vk::PipelineStageFlags srcPipelineStageFlags, vk::PipelineStageFlags dstPipelineStageFlags);
    void Submit();
    ~Command();
};

#endif
