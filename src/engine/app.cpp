#include "app.h"

void Application::Run()
{
    while (!glfwWindowShouldClose(Window::GetWindow())) {
        glfwPollEvents();
        Time::Update();
        m_engine.Render();
    }
}