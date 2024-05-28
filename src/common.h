#ifndef COMMON_H
#define COMMON_H

#ifdef NDEBUG
const bool DEBUG = false;
#else
const bool DEBUG = true;
#endif

// glfw, vulkan library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#if defined(__APPLE__)
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

// std library
#include <iostream>

// external library
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nfd.h>

#include "path.h"

const int MAX_FRAME = 2;

template <typename... T>
void Log(bool debugMode, fmt::terminal_color color, T... args)
{
    if (!debugMode) {
        return;
    }
    spdlog::info(fmt::format(fmt::fg(color), args...));
}

std::string& GetFrameRate();
std::vector<char> FetchCode(const std::string& filename);
std::string nfdOpen(nfdfilteritem_t filterItem);
std::string nfdSave(nfdu8filteritem_t filterItem);
std::string nfdPickFolder();

namespace vkn {
void CheckResult(vk::Result result);
}

#endif