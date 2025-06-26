// Program entry point
// プログラムのエントリーポイント

#include <windows.h>
#include "engine/app.h"
#include "common.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
    AllocConsole();
#endif

    Application app(1920, 1080, "vulkan");

    app.Run();

    return 0;
}