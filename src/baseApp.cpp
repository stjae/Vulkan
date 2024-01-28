#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window_(width, height, wName)
{
    engine_.InitSwapchainImages();
    engine_.SetUp();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(*Window::GetWindow())) {
        glfwPollEvents();
        engine_.DrawUI();
        engine_.UpdateScene();
        engine_.Render();
        GetFramerate();
    }
}

std::string& Application::GetFramerate()
{
    static int frameCount;
    static double delta, currentTime, lastTime;
    static std::string frameRate;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / (float)frameCount << " ms";

        frameRate = title.str();

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;

    return frameRate;
}