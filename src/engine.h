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
#include "imgui.h"

class GraphicsEngine
{
    int frameIndex = 0;
    int maxFrameNumber;

    Device device;
    Swapchain swapchain;
    GraphicsPipeline pipeline;
    Command command;
    MyImGui imgui;
    ImDrawData* imDrawData;

    std::weak_ptr<Scene> scene_;

public:
    GraphicsEngine(std::shared_ptr<Scene>& scene);
    void InitSwapchainImages();
    void Render();
    void RecreateSwapchain();
    void SetupGui();
    void DrawGui();
    ~GraphicsEngine();
};

#endif