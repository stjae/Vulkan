#ifndef ENGINE_H
#define ENGINE_H

#include "../vulkan/command.h"
#include "../vulkan/descriptor.h"
#include "../vulkan/device.h"
#include "../vulkan/instance.h"
#include "../vulkan/logger.h"
#include "../vulkan/pipeline.h"
#include "../vulkan/swapchain.h"
#include "../window.h"
#include "../scene/scene.h"
#include "../scene/camera.h"
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
    void DrawUI();
    void Update();
    void Render();
    ~Engine();
};

#endif