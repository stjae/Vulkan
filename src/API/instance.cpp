#include "instance.h"

Instance::Instance() : logger_(handle_.instance)
{
    vk::ApplicationInfo appInfo(nullptr, 1, nullptr, 1, VK_API_VERSION_1_2);

    vk::InstanceCreateInfo createInfo({}, &appInfo);
    SetExtensions(instanceExtensions_, createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    SetLayers(instanceLayers_, createInfo, debugInfo);

    if (vk::createInstance(&createInfo, nullptr, &handle_.instance) != vk::Result::eSuccess) {
        spdlog::error("failed to create instance");
    }

    logger_.CreateDebugMessenger();
}

void Instance::SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo)
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

    if (debug) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
}

void Instance::SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo)
{
    if (debug) {
        layers.push_back("VK_LAYER_KHRONOS_validation");

        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();

        Logger::SetDebugInfo(debugInfo);
        createInfo.pNext = &debugInfo;
    }
}

void Instance::CreateSurface()
{
    if (glfwCreateWindowSurface(handle_.instance, *Window::GetWindow(), nullptr, &handle_.surface) != VK_SUCCESS) {
        spdlog::error("failed to create window surface");
    }
}

Instance::~Instance()
{
    if (debug) {
        logger_.Destroy();
    }
    handle_.instance.destroySurfaceKHR(handle_.surface);
    handle_.instance.destroy();
}