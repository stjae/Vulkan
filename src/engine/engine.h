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
#pragma comment(lib, "glfw3d.lib")
#pragma comment(lib, "nfdd.lib")
#pragma comment(lib, "assimp-vc143-mtd.lib")
#pragma comment(lib, "yaml-cppd.lib")

#pragma comment(lib, "mono-2.0-sgend.lib")
#pragma comment(lib, "libmonoruntime-sgend.lib")
#pragma comment(lib, "libmono-static-sgend.lib")
#pragma comment(lib, "libmini-sgend.lib")
#pragma comment(lib, "libmonoutilsd.lib")
#pragma comment(lib, "libgcmonosgend.lib")
#pragma comment(lib, "MonoPosixHelperd.lib")
#pragma comment(lib, "eglibd.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "nfd.lib")
#pragma comment(lib, "assimp-vc143-mt.lib")
#pragma comment(lib, "yaml-cpp.lib")

#pragma comment(lib, "mono-2.0-sgen.lib")
#pragma comment(lib, "libmonoruntime-sgen.lib")
#pragma comment(lib, "libmono-static-sgen.lib")
#pragma comment(lib, "libmini-sgen.lib")
#pragma comment(lib, "libmonoutils.lib")
#pragma comment(lib, "libgcmonosgen.lib")
#pragma comment(lib, "MonoPosixHelper.lib")
#pragma comment(lib, "eglib.lib")
#endif

class Engine
{
    vkn::Device m_device;
    vkn::Sync m_sync;
    vkn::Swapchain m_swapchain;
    Viewport m_viewport;
    UI m_imGui;
    Scene m_scene;
    bool m_init;
    std::vector<vk::CommandBuffer> m_commandBuffers;

    void UpdateSwapchain();
    void RecreateSwapchain();

public:
    Engine();
    void Render();
    ~Engine();
};

#endif