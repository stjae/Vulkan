#ifndef _SYNC_H_
#define _SYNC_H_

#include "../common.h"
#include "config.h"
#include "device.h"

class Sync
{
public:
    ~Sync();
    vk::Semaphore CreateVkSemaphore();
    vk::Fence CreateVkFence();
    vk::Fence inFlightFence;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;
};

#endif