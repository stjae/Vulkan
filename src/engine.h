#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "API/device.h"
#include "API/instance.h"
#include "API/logger.h"
#include "API/swapchain.h"
#include "API/window.h"
#include "API/pipeline.h"
#include "API/framebuffer.h"
#include "API/commands.h"
#include "API/sync.h"

class GraphicsEngine
{
public:
    GraphicsEngine(int width, int height, GLFWwindow* window);
    ~GraphicsEngine();
    void Render();

private:
    GLFWwindow* window;
    Instance m_instance;
    Logger m_logger;
    Device m_device;
    Swapchain m_swapchain;
    GraphicsPipeline m_pipeline;
    Framebuffer m_framebuffer;
    Command m_command;
    Sync m_sync;
};

#endif