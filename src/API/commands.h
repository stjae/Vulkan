#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "device.h"
#include "pipeline.h"
#include "../scene.h"

class Command
{
public:
    Command(const Device& device) : device(device) {}
    ~Command();
    void CreateCommandPool(const char* usage);
    void CreateCommandBuffer(vk::CommandBuffer& commandBuffer, const char* usage);
    void RecordDrawCommands(const GraphicsPipeline& pipeline, const vk::CommandBuffer& commandBuffer, uint32_t imageIndex, std::unique_ptr<Scene>& scene, ImDrawData* imDrawData);
    void RecordCopyCommands(const vk::CommandBuffer& commandBuffer, const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, size_t size);

    vk::CommandPool commandPool;

private:
    const Device& device;
};

#endif
