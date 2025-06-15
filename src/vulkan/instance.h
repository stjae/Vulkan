// Wrapper class for Vulkan instance
// Vulkan Instanceのラッパークラス

#ifndef INSTANCE_H
#define INSTANCE_H

#include "../common.h"
#include "../window.h"
#include "debugMessenger.h"

namespace vkn {
class Instance
{
    friend class Device;
    inline static VkSurfaceKHR s_surface;
    inline static vk::Instance s_instance;
    vk::DispatchLoaderDynamic m_dispatchLoaderDynamic;
    DebugMessenger m_debugMessenger;
    std::vector<const char*> m_extensions;
    std::vector<const char*> m_layers;
    bool m_isDebugUtilsAvailable;

    Instance();
    ~Instance();
    void SetExtensions(vk::InstanceCreateInfo& createInfo);
    void SetLayers(vk::InstanceCreateInfo& createInfo);
    void CreateSurface();

public:
    static const VkSurfaceKHR& GetSurface() { return s_surface; }
    static const vk::Instance& GetInstance() { return s_instance; }
};
} // namespace vkn

#endif