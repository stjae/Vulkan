#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window(width, height, wName)
{
    scene = std::make_unique<Scene>();
    engine = std::make_unique<GraphicsEngine>(width, height, window.window, scene);

    engine->Prepare(scene);
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();
        engine->Render(scene);
        GetFramerate();
    }
}

void Application::GetFramerate()
{
    static int frameCount;
    static double currentTime, lastTime, delta;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / frameCount << " ms";
        glfwSetWindowTitle(window.window, title.str().c_str());

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}