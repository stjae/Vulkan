#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"

class Instance
{
public:
    ~Instance();

    static vk::Instance& Get();
    static vk::DispatchLoaderDynamic& Dldi();
    static std::vector<const char*>& Layers();
    static VkSurfaceKHR& Surface();

    void Create();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface();
};

#endif