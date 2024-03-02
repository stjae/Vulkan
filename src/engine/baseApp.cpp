#include "baseApp.h"

void Application::Run()
{
    while (!glfwWindowShouldClose(Window::GetWindow())) {
        glfwPollEvents();
        engine_.DrawUI();
        engine_.Update();
        engine_.Render();
    }
}