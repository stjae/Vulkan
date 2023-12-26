#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window_(width, height, wName)
{
    engine_ = std::make_unique<GraphicsEngine>();
    scene_ = std::make_unique<Scene>();
    engine_->SetupGui();
    engine_->InitSwapchainImages();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(*Window::GetWindow())) {
        glfwPollEvents();
        engine_->DrawGui(scene_);
        engine_->Render(scene_);
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