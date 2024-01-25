#include "common.h"

std::string& GetFrameRate()
{
    static int frameCount;
    static double delta, currentTime, lastTime;
    static std::string frameRate;

    currentTime = glfwGetTime();
    delta = currentTime - lastTime;

    if (delta > 1.0) {
        std::stringstream title;
        title << frameCount << " fps, " << 1000.0f / (float)frameCount << " ms";

        frameRate = title.str();

        lastTime = currentTime;
        frameCount = 0;
    }

    frameCount++;

    return frameRate;
}

std::vector<char> ReadCode(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        spdlog::error("failed to open file: {}", filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), (std::streamsize)fileSize);

    file.close();

    if (!buffer.empty()) {
        Log(debug, fmt::terminal_color::white, "{0} loaded with code length of {1}",
            filename, buffer.size());
    }

    return buffer;
}

std::string LaunchNfd()
{
    NFD_Init();
    std::string filePath;

    nfdchar_t* outPath;
    nfdfilteritem_t filterItem[2] = { { "Image", "jpg,png" }, { "Model", "obj,stl" } };
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, NULL);
    if (result == NFD_OKAY) {
        puts("Success!");
        puts(outPath);
        filePath = outPath;
        NFD_FreePath(outPath);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return filePath;
}

void* AlignedAlloc(size_t dynamicBufferAlignment, size_t bufferSize)
{
#if defined(__APPLE__)
    return aligned_alloc(dynamicBufferAlignment, bufferSize);
#elif defined(_WIN32)
    return _aligned_malloc(bufferSize, dynamicBufferAlignment);
#else
    return nullptr;
#endif
}

void AlignedFree(void* aligned)
{
#if defined(__APPLE__)
    free(aligned);
#elif defined(_WIN32)
    _aligned_free(aligned);
#endif
}