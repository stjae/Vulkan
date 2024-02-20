#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window_(width, height, wName)
{
    engine_.InitSwapchainImages();
    engine_.SetUp();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(Window::GetWindow())) {
        glfwPollEvents();
        engine_.DrawUI();
        engine_.Update();
        engine_.Render();
    }
}