#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "../common.h"

struct Logger {
    Logger(const vk::Instance& vkInstance, const vk::DispatchLoaderDynamic& dldi) : vkInstance(vkInstance), dldi(dldi) {}
    void CreateDebugMessenger();
    static void SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    const vk::Instance& vkInstance;
    const vk::DispatchLoaderDynamic& dldi;

    vk::DebugUtilsMessengerEXT debugMessenger;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData);

#endif