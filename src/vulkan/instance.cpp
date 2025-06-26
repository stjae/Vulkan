#include "instance.h"

namespace vkn {
Instance::Instance()
{
    vk::ApplicationInfo applicationInfo;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo createInfo({}, &applicationInfo);
    SetExtensions(createInfo);

    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    SetLayers(createInfo);
    CHECK_RESULT(vk::createInstance(&createInfo, nullptr, &s_instance));

    m_dispatchLoaderDynamic = vk::DispatchLoaderDynamic(s_instance, vkGetInstanceProcAddr);
    m_debugMessenger.Create(s_instance, m_dispatchLoaderDynamic);
}

void vkn::Instance::SetExtensions(vk::InstanceCreateInfo& createInfo)
{
    m_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    m_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    uint32_t availableExtensionCount = 0;
    CHECK_RESULT(vk::enumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr));
    std::vector<vk::ExtensionProperties> availableInstanceExtensions(availableExtensionCount);
    CHECK_RESULT(vk::enumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableInstanceExtensions.data()));
    for (auto& availableExtension : availableInstanceExtensions) {
        if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            m_isDebugUtilsAvailable = true;
            m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        } else if (strcmp(availableExtension.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0) {
            m_extensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        } else if (strcmp(availableExtension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
            m_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
    }

    createInfo.enabledExtensionCount = m_extensions.size();
    createInfo.ppEnabledExtensionNames = m_extensions.data();
}

void vkn::Instance::SetLayers(vk::InstanceCreateInfo& createInfo)
{
    uint32_t availableLayerCount = 0;
    CHECK_RESULT(vk::enumerateInstanceLayerProperties(&availableLayerCount, nullptr));
    std::vector<vk::LayerProperties> availableInstanceLayers(availableLayerCount);
    CHECK_RESULT(vk::enumerateInstanceLayerProperties(&availableLayerCount, availableInstanceLayers.data()));

    for (auto& availableLayer : availableInstanceLayers) {
        if (strcmp(availableLayer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            m_layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    createInfo.enabledLayerCount = m_layers.size();
    createInfo.ppEnabledLayerNames = m_layers.data();
}

void vkn::Instance::CreateSurface()
{
    if (glfwCreateWindowSurface(s_instance, Window::GetWindow(), nullptr, &s_surface)) {
        spdlog::error("failed to create window surface");
        exit(1);
    }
}

vkn::Instance::~Instance()
{
    if (m_isDebugUtilsAvailable) {
        s_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger.m_messenger, nullptr, m_dispatchLoaderDynamic);
    }
    s_instance.destroySurfaceKHR(s_surface);
    s_instance.destroy();
}
}; // namespace vkn