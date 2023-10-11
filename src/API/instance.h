#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"
#include "config.h"
#include "logger.h"
#include "window.h"

class Instance
{
public:
    ~Instance();

    void CreateInstance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface(GLFWwindow* window);
};

#endif