#ifndef LOGGER_H
#define LOGGER_H

#include "../common.h"

struct Logger
{
    friend class Instance;

    explicit Logger(const vk::Instance& vkInstance) : vkInstance_(vkInstance) {}
    void CreateDebugMessenger();
    static void SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
    void Destroy();

    const vk::Instance& vkInstance_;
    vk::DispatchLoaderDynamic dldi_;
    vk::DebugUtilsMessengerEXT debugMessenger_;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

#endif