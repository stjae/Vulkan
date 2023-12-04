#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "device.h"
#include "pipeline.h"
#include "../mesh.h"

class Command
{
public:
    Command(const Device& device) : device(device) {}
    void CreateCommandPool(const char* usage);
    void AllocateCommandBuffer(vk::CommandBuffer& commandBuffer);
    void RecordDrawCommands(const GraphicsPipeline& pipeline, const vk::CommandBuffer& commandBuffer, uint32_t imageIndex, std::vector<std::unique_ptr<Mesh>>& meshes, ImDrawData* imDrawData);
    void RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);
    void RecordCopyCommands(const vk::Buffer& srcBuffer, const vk::Image& dstImage, const int width, const int height, size_t size);
    void TransitImageLayout(const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void Submit();
    ~Command();

    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;

private:
    const Device& device;
};

#endif
