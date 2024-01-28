#ifndef ENGINE_H
#define ENGINE_H

#include "API/command.h"
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

    bool IsSwapchainOutOfDate(const vk::ResultValue<unsigned int>& waitFrameImage);
    void RecreateSwapchain();

public:
    void InitSwapchainImages();
    void SetUp();
    void UpdateScene();
    void DrawUI();
    void Render();
    ~Engine();
};

#endif