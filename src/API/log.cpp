#include "log.h"

void Log::CreateDebugMessenger()
{
    if (!debug) {
        return;
    }

    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    createInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    createInfo.setPfnUserCallback(DebugCallback);

    debugMessenger = Instance::instance.createDebugUtilsMessengerEXT(createInfo, nullptr, Instance::dldi);
}

void Log::SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    createInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    createInfo.setPfnUserCallback(DebugCallback);
}

Log::~Log()
{
    if (debug) {
        Instance::instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, Instance::dldi);
    }
}

// callback function
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData)
{
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error(fmt::format(fmt::fg(fmt::terminal_color::red), pCallbackData->pMessage));
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn(fmt::format(fmt::fg(fmt::terminal_color::yellow), pCallbackData->pMessage));
        break;

    default:
        spdlog::info(fmt::format(fmt::fg(fmt::terminal_color::black), pCallbackData->pMessage));
        break;
    }

    return VK_FALSE;
}

vk::DebugUtilsMessengerEXT Log::debugMessenger;