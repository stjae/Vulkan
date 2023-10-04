#include "log.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData)
{
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error("validation layer: {}", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn("validation layer: {}", pCallbackData->pMessage);
        break;

    default:
        spdlog::info("validation layer: {}", pCallbackData->pMessage);
        break;
    }

    return vk::False;
}

vk::DebugUtilsMessengerEXT& Log::DebugMessenger()
{
    static vk::DebugUtilsMessengerEXT debugMessenger;
    return debugMessenger;
}

void Log::CreateDebugMessenger()
{
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

    Log::DebugMessenger() = Instance::Get().createDebugUtilsMessengerEXT(createInfo, nullptr, Instance::Dldi());
}

void Log::GetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
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
    Instance::Get().destroyDebugUtilsMessengerEXT(Log::DebugMessenger(), nullptr, Instance::Dldi());
}