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
    void RecordDrawCommands();

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
};

#endif
