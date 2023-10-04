#ifndef _LOG_H_
#define _LOG_H_

#include "../common.h"

struct Log {
    ~Log();

    static vk::DebugUtilsMessengerEXT& DebugMessenger();
    
    void CreateDebugMessenger();
    static void GetDebugInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
};

#endif