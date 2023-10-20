#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "../common.h"
#include "config.h"
#include "device.h"
#include "swapchain.h"
#include "../structs.h"
#include "../scene.h"

class Command
{
public:
    ~Command();
    void CreateCommandPool();
    void CreateCommandBuffer(vk::CommandBuffer& commandBuffer);
    void RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
    void RecordCopyCommands(vk::CommandBuffer commandBuffer, vk::Buffer srcBuffer, vk::Buffer dstBuffer, size_t size);

    vk::CommandPool commandPool;
};

#endif
