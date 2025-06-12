#include <windows.h>
#include "engine/app.h"
#include "common.h"

int main()
{
#if NDEBUG
    HWND hWndConsole = GetConsoleWindow();
    ShowWindow(hWndConsole, SW_HIDE);
#endif

    Application app(1920, 1080, "vulkan");

    app.Run();

    return 0;
}