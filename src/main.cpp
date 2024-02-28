#include "engine/baseApp.h"
#include "common.h"

int main()
{
    Application app(1920, 1080, "vulkan");

    app.Run();

    return 0;
}