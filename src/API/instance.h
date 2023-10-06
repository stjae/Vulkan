#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"
#include "log.h"
#include "window.h"

class Instance
{
public:
    ~Instance();

    void CreateInstance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface();

    static vk::Instance instance;
    static vk::DispatchLoaderDynamic dldi;
    static std::vector<const char*> extensions;
    static std::vector<const char*> layers;
    static VkSurfaceKHR surface;
};

#endif