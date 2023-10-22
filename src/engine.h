#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "API/device.h"
#include "API/instance.h"
#include "API/logger.h"
#include "API/swapchain.h"
#include "API/window.h"
#include "API/pipeline.h"
#include "API/commands.h"
#include "API/sync.h"
#include "scene.h"

class GraphicsEngine
{
public:
    GraphicsEngine(int width, int height, GLFWwindow* window);
    ~GraphicsEngine();
    void Prepare(Scene* scene);
    void Render(Scene* scene);
    void RecreateSwapchain();

private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    Instance m_instance;
    Logger m_logger;
    Device m_device;
    Swapchain m_swapchain;
    GraphicsPipeline m_pipeline;
    Command m_command;
    Sync m_sync;
};

#endif