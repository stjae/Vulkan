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
#include "viewport.h"

class Engine
{
    size_t frameIndex_ = 0;

    Device device_;
    Swapchain swapchain_;
    Viewport viewport_;
    MyImGui imgui_;
    ImDrawData* imDrawData_{};

    std::unique_ptr<Scene> scene_;

public:
    void InitSwapchainImages();
    void Render(std::unique_ptr<Scene>& scene);
    void RecreateSwapchain();
    void SetupGui();
    void DrawGui(std::unique_ptr<Scene>& scene);
    ~Engine();
};

#endif