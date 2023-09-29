#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#include "common.h"
#include "vkDebug.h"
#include "vkUtil.h"

class HelloTriangleApplication
{
public:
    HelloTriangleApplication(const int width, const int height, bool enableValidationLayers)
        : m_width(width),
          m_height(height)
    {
        s_baseAppInfo.enableValidationLayers = enableValidationLayers;
    }

    void run();

private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void CleanUp();

    void DrawFrame();

private:
    const int m_width, m_height;

    GLFWwindow* m_window;

    BaseAppInfo& s_baseAppInfo = BaseAppInfo::Get();

    VkUtil& s_vkUtil = VkUtil::Get();
    VkDebug& s_vkDebug = VkDebug::Get();
};

#endif