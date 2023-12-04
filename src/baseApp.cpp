#include "baseApp.h"

Application::Application(const int width, const int height, const char* wName)
    : window(width, height, wName), camera(window.window)
{
    scene = std::make_unique<Scene>();
    engine = std::make_unique<GraphicsEngine>(width, height, window.window, scene);

    imgui.Setup(engine);

    engine->InitSwapchainImages();
    engine->Prepare(scene);
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();

        if (camera.isControllable) {
            camera.Update(window);
        }

        imgui.Draw(camera, scene, window.window);
        ImDrawData* draw_data = ImGui::GetDrawData();

        if (ImGui::IsKeyPressed(ImGuiKey_X))
            scene->meshes.pop_back();

        engine->Render(scene, draw_data, camera);
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