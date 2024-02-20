#include "logger.h"

void Logger::CreateDebugMessenger()
{
    if (!debug) {
        return;
    }

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo({}, severityFlags, messageTypeFlags, DebugCallback);

    dldi_ = vk::DispatchLoaderDynamic(vkInstance_, vkGetInstanceProcAddr);
    debugMessenger_ = vkInstance_.createDebugUtilsMessengerEXT(debugMessengerCreateInfo, nullptr, dldi_);
}

void Logger::SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    createInfo.pfnUserCallback = DebugCallback;
}

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

void Logger::Destroy()
{
    vkInstance_.destroyDebugUtilsMessengerEXT(debugMessenger_, nullptr, dldi_);
}