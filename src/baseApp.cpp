#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window_(width, height, wName)
{
    scene_ = std::make_unique<Scene>();
    engine_ = std::make_unique<GraphicsEngine>(scene_);
    scene_->Prepare();
    engine_->SetupGui();
    engine_->InitSwapchainImages();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(*Window::GetWindow())) {
        glfwPollEvents();
        engine_->DrawGui();
        engine_->Render();
        GetFramerate();
    }
}

void Application::GetFramerate()
{
    static int frameCount;
    static double delta, currentTime, lastTime;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / frameCount << " ms";
        glfwSetWindowTitle(*Window::GetWindow(), title.str().c_str());

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}