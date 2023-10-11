#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "../common.h"
#include "config.h"
#include "device.h"
#include "swapchain.h"

class Command
{
public:
    ~Command();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

    vk::CommandPool commandPool;
    vk::CommandBuffer mainCommandBuffer;
};

#endif
