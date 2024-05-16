#include "app.h"

void Application::Run()
{
    vkn::Device::Get().graphicsQueue.waitIdle();
    while (!glfwWindowShouldClose(Window::GetWindow())) {
        glfwPollEvents();
        m_engine.Render();
    }
}