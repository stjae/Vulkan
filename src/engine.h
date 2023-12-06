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
#include "camera.h"

class GraphicsEngine
{
public:
    GraphicsEngine(GLFWwindow* window, std::unique_ptr<Scene>& scene);
    void InitSwapchainImages();
    void Render(std::unique_ptr<Scene>& scene, ImDrawData* imDrawData);
    void RecreateSwapchain();
    ~GraphicsEngine();

    GLFWwindow* window;

    Device device;
    Swapchain swapchain;
    GraphicsPipeline pipeline;
    Command command;

    int frameIndex = 0;
    int maxFrameNumber;
};

#endif