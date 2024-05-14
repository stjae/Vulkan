#ifndef DEBUGMESSENGER_H
#define DEBUGMESSENGER_H

#include "../common.h"

namespace vkn {
class DebugMessenger
{
    friend class Instance;

    void Create(const vk::Instance& instance);
    void SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    vk::DispatchLoaderDynamic m_loader;
    vk::DebugUtilsMessengerEXT m_messenger;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
} // namespace vkn

#endif