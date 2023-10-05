#include "instance.h"

void Instance::Create()
{
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(vk::makeApiVersion(0, 1, 0, 0));

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);

    std::vector<const char*> extensions;
    SetExtensions(extensions, createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    SetLayers(Layers(), createInfo, debugInfo);

    if (vk::createInstance(&createInfo, nullptr, &Instance::Get()) != vk::Result::eSuccess) {
        spdlog::error("failed to create instance");
    }

    Instance::Dldi() = vk::DispatchLoaderDynamic(Instance::Get(), vkGetInstanceProcAddr);
}

void Instance::SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo)
{
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    createInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);

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

        Log::GetDebugInfo(debugInfo);
        createInfo.pNext = &debugInfo;
    }
}

void Instance::CreateSurface()
{
    if (glfwCreateWindowSurface(Instance::Get(), Window::Get(), nullptr, &Surface()) != VK_SUCCESS) {
        spdlog::error("failed to create window surface");
    }
}

Instance::~Instance()
{
    Instance::Get().destroySurfaceKHR(Surface());
    Instance::Get().destroy();
}

// getter
vk::Instance& Instance::Get()
{
    static vk::Instance instance;
    return instance;
}

vk::DispatchLoaderDynamic& Instance::Dldi()
{
    static vk::DispatchLoaderDynamic dldi;
    return dldi;
}

std::vector<const char*>& Instance::Layers()
{
    static std::vector<const char*> layers;
    return layers;
}

VkSurfaceKHR& Instance::Surface()
{
    static VkSurfaceKHR surface;
    return surface;
}