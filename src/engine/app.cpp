#include "app.h"

void Application::Run()
{
    while (!glfwWindowShouldClose(Window::GetWindow())) {
        glfwPollEvents();
        // m_engine.DrawUI();
        m_engine.Render();
    }
}