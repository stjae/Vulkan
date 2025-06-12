#include "debugMessenger.h"

namespace vkn {
void DebugMessenger::Create(const vk::Instance& instance, const vk::DispatchLoaderDynamic& dispatchLoaderDynamic)
{
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
    debugUtilsCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    debugUtilsCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debugUtilsCreateInfo.pfnUserCallback = DebugCallback;

    m_messenger = instance.createDebugUtilsMessengerEXT(debugUtilsCreateInfo, nullptr, dispatchLoaderDynamic);
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData)
{
    std::string message = pCallbackData->pMessage;

    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error(message);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn(message);
        break;

    default:
        spdlog::info(message);
        break;
    }

    return VK_FALSE;
}
}; // namespace vkn
