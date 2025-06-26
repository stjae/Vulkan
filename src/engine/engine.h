// Execute the update and rendering of the viewport, UI, and scene
// ビューポート、UI、シーンの更新およびレンダリングを実行

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

#pragma comment(lib, "vulkan-1.lib")
#ifdef _DEBUG
#define PATH "debug/"
#else
#define PATH ""
#endif
#pragma comment(lib, PATH "glfw3.lib")
#pragma comment(lib, PATH "nfd.lib")
#pragma comment(lib, PATH "assimp-vc143-mt.lib")
#pragma comment(lib, PATH "yaml-cpp.lib")
#pragma comment(lib, PATH "mono-2.0-sgen.lib")
#pragma comment(lib, PATH "libmonoruntime-sgen.lib")
#pragma comment(lib, PATH "libmono-static-sgen.lib")
#pragma comment(lib, PATH "libmini-sgen.lib")
#pragma comment(lib, PATH "libmonoutils.lib")
#pragma comment(lib, PATH "libgcmonosgen.lib")
#pragma comment(lib, PATH "MonoPosixHelper.lib")
#pragma comment(lib, PATH "eglib.lib")
#pragma comment(lib, PATH "ktx.lib")
#pragma comment(lib, PATH "ktx_read.lib")

class Engine
{
    vkn::Device m_device;
    vkn::Sync m_sync;
    vkn::Swapchain m_swapchain;
    Viewport m_viewport;
    UI m_imGui;
    Scene m_scene;
    bool m_init = true;
    int m_currentFrame = 0;

    void UpdateSwapchain(const vk::CommandBuffer& commandBuffer);
    void RecreateSwapchain(const vk::CommandBuffer& commandBuffer);

public:
    Engine();
    void Render();
    ~Engine();
};

#endif
