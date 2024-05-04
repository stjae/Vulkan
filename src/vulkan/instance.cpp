#include "instance.h"

vkn::Instance::Instance() : logger_(instanceBundle_.instance)
{
    vk::ApplicationInfo appInfo(nullptr, 1, nullptr, 1, VK_API_VERSION_1_2);

    vk::InstanceCreateInfo createInfo({}, &appInfo);
    SetExtensions(instanceExtensions_, createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    SetLayers(instanceLayers_, createInfo, debugInfo);

    vkn::CheckResult(vk::createInstance(&createInfo, nullptr, &instanceBundle_.instance));

    logger_.CreateDebugMessenger();
}

void vkn::Instance::SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo)
{
#if defined(__APPLE__)
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    createInfo.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    uint32_t count = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

    for (uint32_t i = 0; i < count; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (DEBUG) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
}

void vkn::Instance::SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo)
{
    if (DEBUG) {
        layers.push_back("VK_LAYER_KHRONOS_validation");

        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();

        Logger::SetDebugInfo(debugInfo);
        createInfo.pNext = &debugInfo;
    }
}

void vkn::Instance::CreateSurface()
{
    if (glfwCreateWindowSurface(instanceBundle_.instance, Window::GetWindow(), nullptr, &instanceBundle_.surface)) {
        spdlog::error("failed to create window surface");
    }
}

vkn::Instance::~Instance()
{
    if (DEBUG) {
        logger_.Destroy();
    }
    instanceBundle_.instance.destroySurfaceKHR(instanceBundle_.surface);
    instanceBundle_.instance.destroy();
}