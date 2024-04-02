#ifndef INSTANCE_H
#define INSTANCE_H

#include "../common.h"
#include "../window.h"
#include "logger.h"

namespace vkn {
struct InstanceBundle
{
    VkSurfaceKHR surface;
    vk::Instance instance;
};

class Instance
{
    friend class Device;

    Logger logger_;
    inline static InstanceBundle instanceBundle_;

    std::vector<const char*> instanceExtensions_;
    std::vector<const char*> instanceLayers_;

    Instance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface();
    ~Instance();

public:
    static const InstanceBundle& GetBundle() { return instanceBundle_; }
};
} // namespace vkn

#endif