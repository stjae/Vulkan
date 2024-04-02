#include "app.h"

void Application::Run()
{
    while (!glfwWindowShouldClose(Window::GetWindow())) {
        glfwPollEvents();
        engine_.DrawUI();
        engine_.Render();
    }
}