#include "vkDebug.h"

void VkDebug::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{

    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)DebugCallback;
}

void VkDebug::SetupDebugMessenger()
{

    if (!s_baseAppInfo.enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;

    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr) != VK_SUCCESS) {
        spdlog::error("failed to set up debug messenger");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebug::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn("validation layer: {}", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error("validation layer: {}", pCallbackData->pMessage);
        break;
    
    default:
        spdlog::info("validation layer: {}", pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

VkResult VkDebug::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator)
{

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_vkUtil.instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(s_vkUtil.instance, pCreateInfo, pAllocator, &debugUtilsMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VkDebug::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator)
{

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_vkUtil.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(s_vkUtil.instance, debugUtilsMessenger, pAllocator);
    }
}

bool VkDebug::CheckValidationLayerSupport()
{

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : s_baseAppInfo.validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}