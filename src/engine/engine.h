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
#include "ui.h"
#include "viewport.h"

class Engine
{
    vk::CommandPool commandPool_;
    vk::CommandBuffer commandBuffer_;

    size_t frameIndex_ = 0;
    Device device_;
    Swapchain swapchain_;
    Viewport viewport_;
    UI imgui_;
    std::unique_ptr<Scene> scene_;

    void UpdateSwapchain();
    void RecreateSwapchain();

public:
    Engine();
    void InitSwapchainImages();
    void DrawUI();
    void Render();
    ~Engine();
};

#endif