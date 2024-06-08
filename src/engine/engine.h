#ifndef ENGINE_H
#define ENGINE_H

#include "../vulkan/command.h"
#include "../vulkan/descriptor.h"
#include "../vulkan/device.h"
#include "../vulkan/instance.h"
#include "../vulkan/debugMessenger.h"
#include "../vulkan/pipeline.h"
#include "../vulkan/swapchain.h"
#include "../window.h"
#include "../scene/scene.h"
#include "../scene/camera.h"
#include "ui.h"
#include "viewport.h"

class Engine
{
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;

    vkn::Device m_device;
    vkn::Sync m_sync;
    vkn::Swapchain m_swapchain;
    Viewport m_viewport;
    UI m_imGui;
    std::unique_ptr<Scene> m_scene;
    bool m_init;

    void UpdateSwapchain();
    void RecreateSwapchain();

public:
    Engine();
    void Render();
    ~Engine();
};

#endif