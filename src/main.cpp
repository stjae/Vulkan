#include "engine/app.h"
#include "common.h"

int main()
{
    Application app(1920 * 1.5, 1080 * 1.5, "vulkan");

    app.Run();

    return 0;
}