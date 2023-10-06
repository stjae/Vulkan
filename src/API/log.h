#ifndef _LOG_H_
#define _LOG_H_

#include "../common.h"
#include "instance.h"

struct Log {
    ~Log();

    void CreateDebugMessenger();
    static void SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    static vk::DebugUtilsMessengerEXT debugMessenger;
};

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData);

#endif