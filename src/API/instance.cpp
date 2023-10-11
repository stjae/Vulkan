#include "instance.h"

void Instance::CreateInstance()
{
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(vk::makeApiVersion(0, 1, 0, 0));

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);
    SetExtensions(instanceExtensions, createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    SetLayers(instanceLayers, createInfo, debugInfo);

    if (vk::createInstance(&createInfo, nullptr, &instance) != vk::Result::eSuccess) {
        spdlog::error("failed to create instance");
    }

    dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
}

void Instance::SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo)
{
#if defined(__APPLE__)
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    createInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    uint32_t count = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

    for (uint32_t i = 0; i < count; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (debug) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.setEnabledExtensionCount((uint32_t)extensions.size());
    createInfo.setPpEnabledExtensionNames(extensions.data());
}

void Instance::SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo)
{
    if (debug) {
        layers.push_back("VK_LAYER_KHRONOS_validation");

        createInfo.setEnabledLayerCount((uint32_t)layers.size());
        createInfo.setPpEnabledLayerNames(layers.data());

        Logger::SetDebugInfo(debugInfo);
        createInfo.pNext = &debugInfo;
    }
}

void Instance::CreateSurface(GLFWwindow* window)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        spdlog::error("failed to create window surface");
    }
}

Instance::~Instance()
{
    instance.destroySurfaceKHR(surface);
    instance.destroy();
}