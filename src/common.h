#ifndef COMMON_H
#define COMMON_H

#ifdef NDEBUG
const bool debugMode = false;
#else
const bool debugMode = true;
#endif

// glfw, vulkan library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#if defined(__APPLE__)
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

// std library
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>

// external library
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <nfd.h>
#include <stb/stb_image.h>
#include "path.h"

template <typename... T>
void Log(bool isDebugMode, fmt::terminal_color color, T... args)
{
    if (!isDebugMode) {
        return;
    }
    spdlog::info(fmt::format(fmt::fg(color), args...));
}

std::string& GetFrameRate();
std::vector<char> FetchCode(const std::string& filename);
std::string nfdOpen(nfdfilteritem_t filterItem);
std::string nfdSave(nfdu8filteritem_t filterItem);
void* AlignedAlloc(size_t dynamicBufferAlignment, size_t bufferSize);
void AlignedFree(void* aligned);

namespace vkn {
void CheckResult(vk::Result result);
}

#endif