#include "instance.h"

Instance::Instance()
{
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("Hello Triangle");

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);
    createInfo.setEnabledExtensionCount(1);

    const char extensionName[] = "VK_KHR_portability_enumeration";
    const char* pExtensionName = extensionName;
    createInfo.setPEnabledExtensionNames(pExtensionName);
    createInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);

    if (vk::createInstance(&createInfo, nullptr, &m_instance) != vk::Result::eSuccess) {
        spdlog::error("failed to create instance");
    }
}

Instance::~Instance()
{
    vkDestroyInstance(m_instance, nullptr);
}
