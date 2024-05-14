#include "debugMessenger.h"

namespace vkn {
void DebugMessenger::Create(const vk::Instance& instance)
{
    if (!DEBUG) {
        return;
    }

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo({}, severityFlags, messageTypeFlags, DebugCallback);

    m_loader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    m_messenger = instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo, nullptr, m_loader);
}

void DebugMessenger::SetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
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
}; // namespace vkn
