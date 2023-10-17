#ifndef _SYNC_H_
#define _SYNC_H_

#include "../common.h"
#include "config.h"
#include "device.h"

class Sync
{
public:
    vk::Semaphore MakeSemaphore();
    vk::Fence MakeFence();
    int maxFramesInFlight;
    int frameNumber;
};

#endif