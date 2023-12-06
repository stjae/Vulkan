#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window(width, height, wName)
{
    scene = std::make_unique<Scene>();
    engine = std::make_unique<GraphicsEngine>(width, height, window.window, scene);
    scene->Prepare(engine->device);

    imgui.Setup(engine);

    engine->InitSwapchainImages();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();

        imgui.Draw(scene, window.window);
        ImDrawData* draw_data = ImGui::GetDrawData();

        engine->Render(scene, draw_data);
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
        glfwSetWindowTitle(window.window, title.str().c_str());

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;
}