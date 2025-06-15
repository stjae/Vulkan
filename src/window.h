// Class for creating windows, receiving mouse and keyboard input, and handling resize events using GLFW
// GLFWを利用してウィンドウの生成、マウスやキーボードの入力取得、リサイズイベントの処理を行うクラス

#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"
#include "keycode.h"

class Window
{
    inline static GLFWwindow* s_glfwWindow;

public:
    inline static bool s_resized = false;

    Window(int width, int height, const char* title);
    static GLFWwindow* GetWindow() { return s_glfwWindow; }
    static double GetMousePosX();
    static double GetMousePosY();
    static float GetMousePosNormalizedX();
    static float GetMousePosNormalizedY();
    static bool IsKeyDown(Keycode keycode);
    static bool IsMouseButtonDown(MouseButton mouseButton);
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) { s_resized = true; }
    ~Window();
};

#endif