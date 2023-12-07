#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"
#include "logger.h"
#include "window.h"

class Instance
{
    Logger logger;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;

public:
    Instance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface();
    ~Instance();

    const char** GetInstanceExtensions() { return instanceExtensions.data(); }
    size_t GetNumInstanceExtensions() { return instanceExtensions.size(); }
    const char** GetInstanceLayers() { return instanceLayers.data(); }
    size_t GetNumInstanceLayers() { return instanceLayers.size(); }
    static VkSurfaceKHR& GetSurface()
    {
        static VkSurfaceKHR vkSurface;
        return vkSurface;
    }
    static vk::Instance& GetInstance()
    {
        static vk::Instance vkInstance;
        return vkInstance;
    }
};

#endif