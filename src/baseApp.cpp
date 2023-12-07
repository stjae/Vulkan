#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window(width, height, wName)
{
    scene = std::make_unique<Scene>();
    engine = std::make_unique<GraphicsEngine>(scene);
    scene->Prepare();
    engine->SetupGui();
    engine->InitSwapchainImages();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(*Window::GetWindow())) {
        glfwPollEvents();
        engine->DrawGui();
        engine->Render();
        GetFramerate();
    }
}

void Application::GetFramerate()
{
    static int frameCount;
    static double currentTime, lastTime;

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