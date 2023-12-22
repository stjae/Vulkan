#ifndef ENGINE_H
#define ENGINE_H

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
    int frameIndex_ = 0;
    int maxFrameNumber_;

    Device device_;
    Swapchain swapchain_;
    GraphicsPipeline pipeline_;
    Command command_;
    MyImGui imgui_;
    ImDrawData* imDrawData_{};

    std::unique_ptr<Scene> scene_;

public:
    GraphicsEngine();
    void InitSwapchainImages();
    void Render(std::unique_ptr<Scene>& scene);
    void RecreateSwapchain();
    void SetupGui();
    void DrawGui(std::unique_ptr<Scene>& scene);
    ~GraphicsEngine();
};

#endif