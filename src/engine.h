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
    GraphicsEngine(int width, int height, GLFWwindow* window, std::unique_ptr<Scene>& scene);
    void InitSwapchainImages();
    void CreateDepthImage();
    void UpdateFrame(uint32_t imageIndex, Camera& camera, std::unique_ptr<Scene>& scene);
    void Prepare(std::unique_ptr<Scene>& scene);
    void Render(std::unique_ptr<Scene>& scene, ImDrawData* imDrawData, Camera& camera);
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

    UBO ubo;
};

#endif