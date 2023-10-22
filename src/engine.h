#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "API/commands.h"
#include "API/descriptor.h"
#include "API/device.h"
#include "API/instance.h"
#include "API/logger.h"
#include "API/pipeline.h"
#include "API/swapchain.h"
#include "API/window.h"
#include "scene.h"

class GraphicsEngine
{
public:
    GraphicsEngine(int width, int height, GLFWwindow* window);
    void UpdateFrame(uint32_t imageIndex);
    void Prepare(Scene* scene);
    void Render(Scene* scene);
    void RecreateSwapchain();
    ~GraphicsEngine();

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

    int m_frameIndex = 0;
    int m_maxFrameNumber;
};

#endif