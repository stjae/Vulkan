#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "API/device.h"
#include "API/instance.h"
#include "API/log.h"
#include "API/swapchain.h"
#include "API/window.h"
#include "API/pipeline.h"
#include "API/framebuffer.h"
#include "common.h"

class Application
{
public:
    Application(const int width, const int height, const char* wName);

    void run();

private:
    void InitVulkan();
    void MainLoop();

    Window m_window;
    Instance m_instance;
    Log m_log;
    Device m_device;
    Swapchain m_swapchain;
    GraphicsPipeline m_pipeline;
    Framebuffer m_framebuffer;
};

#endif