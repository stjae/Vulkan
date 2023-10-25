#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"
#include "logger.h"
#include "window.h"

class Instance
{
public:
    Instance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface(GLFWwindow* window);
    ~Instance();

    Logger logger;

    vk::Instance vkInstance;
    vk::DispatchLoaderDynamic dldi;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;
    VkSurfaceKHR vkSurface;
};

#endif