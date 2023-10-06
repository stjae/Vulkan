#include "instance.h"

void Instance::CreateInstance()
{
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(vk::makeApiVersion(0, 1, 0, 0));

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);
    SetExtensions(extensions, createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    SetLayers(layers, createInfo, debugInfo);

    if (vk::createInstance(&createInfo, nullptr, &Instance::instance) != vk::Result::eSuccess) {
        spdlog::error("failed to create instance");
    }

    Instance::dldi = vk::DispatchLoaderDynamic(Instance::instance, vkGetInstanceProcAddr);
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

    for (int i = 0; i < count; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (debug) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.setEnabledExtensionCount(extensions.size());
    createInfo.setPpEnabledExtensionNames(extensions.data());
}

void Instance::SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo)
{
    if (debug) {
        layers.push_back("VK_LAYER_KHRONOS_validation");

        createInfo.setEnabledLayerCount(layers.size());
        createInfo.setPpEnabledLayerNames(layers.data());

        Log::SetDebugInfo(debugInfo);
        createInfo.pNext = &debugInfo;
    }
}

void Instance::CreateSurface()
{
    if (glfwCreateWindowSurface(Instance::instance, Window::window, nullptr, &surface) != VK_SUCCESS) {
        spdlog::error("failed to create window surface");
    }
}

Instance::~Instance()
{
    Instance::instance.destroySurfaceKHR(surface);
    Instance::instance.destroy();
}

vk::Instance Instance::instance;
vk::DispatchLoaderDynamic Instance::dldi;
std::vector<const char*> Instance::extensions;
std::vector<const char*> Instance::layers;
VkSurfaceKHR Instance::surface;