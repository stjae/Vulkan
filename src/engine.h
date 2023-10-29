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
    GraphicsEngine(int width, int height, GLFWwindow* window, std::unique_ptr<Scene>& scene);
    void UpdateFrame(uint32_t imageIndex);
    void Prepare(std::unique_ptr<Scene>& scene);
    void Render(std::unique_ptr<Scene>& scene, ImDrawData* imDrawData);
    void RecreateSwapchain();
    ~GraphicsEngine();

    GLFWwindow* window;
    int width;
    int height;

    Device device;
    Swapchain swapchain;
    GraphicsPipeline pipeline;
    Command command;

    int frameIndex = 0;
    int maxFrameNumber;
};

#endif