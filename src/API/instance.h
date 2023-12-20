#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"
#include "logger.h"
#include "window.h"

class Instance
{
    Logger logger_;
    std::vector<const char*> instanceExtensions_;
    std::vector<const char*> instanceLayers_;

    inline static InstanceHandle handle_;

public:
    Instance();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);
    void CreateSurface();
    ~Instance();

    static const InstanceHandle& GetHandle() { return handle_; }

    const char** GetInstanceExtensions() { return instanceExtensions_.data(); }
    size_t GetNumInstanceExtensions() { return instanceExtensions_.size(); }
    const char** GetInstanceLayers() { return instanceLayers_.data(); }
    size_t GetNumInstanceLayers() { return instanceLayers_.size(); }
};

#endif