#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : m_window(width, height, wName)
{
    engine = new GraphicsEngine(m_window.width, m_window.height, m_window.window);
}

void Application::Run()
{
    while (!glfwWindowShouldClose(m_window.window)) {
        glfwPollEvents();
        engine->Render();
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
        glfwSetWindowTitle(m_window.window, title.str().c_str());

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}

Application::~Application()
{
    delete engine;
}