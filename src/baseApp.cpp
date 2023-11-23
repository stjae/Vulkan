#include "baseApp.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
        camera->isControllable = !camera->isControllable;

        if (camera->isControllable) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        camera->isInitial = true;
    }
}

Application::Application(const int width, const int height, const char* wName)
    : window(width, height, wName), camera(window.window)
{
    scene = std::make_unique<Scene>();
    engine = std::make_unique<GraphicsEngine>(width, height, window.window, scene);

    imgui.Setup(engine);

    engine->InitSwapchainImages();
    engine->Prepare(scene);

    glfwSetKeyCallback(window.window, KeyCallback);
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();

        if (camera.isControllable) {
            camera.Update(window);
        }

        imgui.Draw(camera, scene);
        ImDrawData* draw_data = ImGui::GetDrawData();

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