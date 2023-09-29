#ifndef _BASEAPPINFO_H_
#define _BASEAPPINFO_H_

#include "common.h"

struct BaseAppInfo {

    static BaseAppInfo& Get()
    {
        static BaseAppInfo info;
        return info;
    }

    bool enableValidationLayers;
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

private:
    BaseAppInfo() {}
};

#endif