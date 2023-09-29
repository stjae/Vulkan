#ifndef _VKDEBUG_H_
#define _VKDEBUG_H_

#include "common.h"
#include "baseAppInfo.h"
#include "vkUtil.h"

class VkDebug
{
public:
    static VkDebug& Get()
    {
        static VkDebug debug;
        return debug;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void SetupDebugMessenger();
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator);
    void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);
    bool CheckValidationLayerSupport();

    VkDebugUtilsMessengerEXT debugUtilsMessenger;

private:
    VkDebug() {}
    BaseAppInfo& s_baseAppInfo = BaseAppInfo::Get();
    VkUtil& s_vkUtil = VkUtil::Get();
};

#endif