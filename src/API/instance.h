#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common.h"

class Instance
{
public:
    Instance(bool enableValidationLayers) : m_enableValidationLayers(enableValidationLayers){};
    ~Instance();

    static vk::Instance& Get();
    static vk::DispatchLoaderDynamic& Dldi();

    void Create();
    void SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo);
    void SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo);

    bool m_enableValidationLayers;
};

#endif