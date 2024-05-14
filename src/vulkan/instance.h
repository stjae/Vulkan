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
    DebugMessenger m_debugMessenger;
    std::vector<const char*> m_extensions;
    std::vector<const char*> m_layers;

    Instance();
    ~Instance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface();

public:
    static const VkSurfaceKHR& GetSurface() { return s_surface; }
    static const vk::Instance& GetInstance() { return s_instance; }
};
} // namespace vkn

#endif