// Handling user input, updating time, and executing the engine's rendering loop
// ユーザー入力の処理、時間の更新、エンジンのレンダリングループの実行

#ifndef BASEAPP_H
#define BASEAPP_H

#include "engine.h"
#include "../myTime.h"
#include "../common.h"

class Application
{
    Window m_window;
    Engine m_engine;

public:
    Application(int width, int height, const char* title) : m_window(width, height, title) {}

    void Run();
};

#endif