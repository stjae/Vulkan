#include "instance.h"

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

void Instance::Create()
{
    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(vk::makeApiVersion(0, 1, 0, 0));

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);

    std::vector<const char*> extensions;
    SetExtensions(extensions, createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    std::vector<const char*> layers;
    SetLayers(layers, createInfo, debugInfo);

    if (vk::createInstance(&createInfo, nullptr, &Instance::Get()) != vk::Result::eSuccess) {
        spdlog::error("failed to create instance");
    }

    Instance::Dldi() = vk::DispatchLoaderDynamic(Instance::Get(), vkGetInstanceProcAddr);
}

void Instance::SetExtensions(std::vector<const char*>& extensions, vk::InstanceCreateInfo& createInfo)
{
    extensions.push_back("VK_KHR_portability_enumeration");
    createInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);

    uint32_t count = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

    for (int i = 0; i < count; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (m_enableValidationLayers) {
        extensions.push_back("VK_EXT_debug_utils");
    }

    createInfo.setEnabledExtensionCount(extensions.size());
    createInfo.setPpEnabledExtensionNames(extensions.data());
}

void Instance::SetLayers(std::vector<const char*>& layers, vk::InstanceCreateInfo& createInfo, vk::DebugUtilsMessengerCreateInfoEXT& debugInfo)
{
    if (m_enableValidationLayers) {
        layers.push_back("VK_LAYER_KHRONOS_validation");

        createInfo.setEnabledLayerCount(layers.size());
        createInfo.setPpEnabledLayerNames(layers.data());

        Log::GetDebugInfo(debugInfo);
        createInfo.pNext = &debugInfo;
    }
}

Instance::~Instance()
{
    vkDestroyInstance(Instance::Get(), nullptr);
}