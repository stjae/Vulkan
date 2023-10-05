#ifndef _LOG_H_
#define _LOG_H_

#include "../common.h"

struct Log {
    ~Log();

    static vk::DebugUtilsMessengerEXT& DebugMessenger();

    void CreateDebugMessenger();
    static void GetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData);

#endif