#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "device.h"
#include "pipeline.h"
#include "../structs.h"
#include "../scene.h"

class Command
{
public:
    Command(const Device& device) : device(device) {}
    ~Command();
    void CreateCommandPool();
    void CreateCommandBuffer(vk::CommandBuffer& commandBuffer);
    void RecordDrawCommands(GraphicsPipeline& pipeline, vk::CommandBuffer commandBuffer, uint32_t imageIndex, std::unique_ptr<Scene>& scene);
    void RecordCopyCommands(vk::CommandBuffer commandBuffer, vk::Buffer srcBuffer, vk::Buffer dstBuffer, size_t size);

    vk::CommandPool commandPool;

private:
    const Device& device;
};

#endif
