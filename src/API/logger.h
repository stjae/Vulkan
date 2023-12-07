#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "../common.h"

struct Logger
{
    Logger(const vk::Instance& vkInstance) : vkInstance_(vkInstance) {}
    void CreateDebugMessenger();
    static void SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
    void Destroy();

private:
    const vk::Instance& vkInstance_;
    vk::DispatchLoaderDynamic dldi_;
    vk::DebugUtilsMessengerEXT debugMessenger_;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData);

#endif