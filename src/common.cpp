#include "common.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <fstream>

std::vector<char> FetchCode(const std::string& filename)
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
        Log(DEBUG, fmt::terminal_color::white, "{0} loaded with code length of {1}",
            filename, buffer.size());
    }

    return buffer;
}

std::string nfdOpen(nfdfilteritem_t filterItem)
{
    NFD_Init();
    std::string pathStr;

    nfdchar_t* openPath;
    nfdresult_t result = NFD_OpenDialog(&openPath, &filterItem, 1, nullptr);
    if (result == NFD_OKAY) {
        puts("Success!");
        puts(openPath);
        pathStr = openPath;
        NFD_FreePath(openPath);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return pathStr;
}

std::string nfdSave(nfdu8filteritem_t filterItem)
{
    NFD_Init();
    std::string pathStr;
    nfdu8char_t* savePath;

    nfdresult_t result = NFD_SaveDialog(&savePath, &filterItem, 1, nullptr, nullptr);
    if (result == NFD_OKAY) {
        puts("Success!");
        puts(savePath);
        pathStr = savePath;
        NFD_FreePath(savePath);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return pathStr;
}

std::string nfdPickFolder()
{
    NFD_Init();
    std::string pathStr;
    nfdu8char_t* folderPath;

    nfdresult_t result = NFD_PickFolderU8(&folderPath, nullptr);
    if (result == NFD_OKAY) {
        puts("Success!");
        puts((const char*)folderPath);
        pathStr = reinterpret_cast<const char* const>(folderPath);
        NFD_FreePathU8(folderPath);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return pathStr;
}

void vkn::CheckResult(vk::Result result, const char* file, int line)
{
    if (result != vk::Result::eSuccess) {
        Log(DEBUG, fmt::terminal_color::red, "{0}, in file {1}, line {2}", vk::to_string(result), file, line);
    }
}
