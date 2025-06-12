#ifndef COMMON_H
#define COMMON_H

#ifdef NDEBUG
const bool DEBUG = false;
#else
const bool DEBUG = true;
#endif

// glfw, vulkan library
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define NOMINMAX
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

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
#include <nfd/nfd.h>

const int MAX_FRAMES_IN_FLIGHT = 2;
template <typename... T>
void Log(bool debugMode, fmt::terminal_color color, T... args)
{
    if (!debugMode) {
        return;
    }
    spdlog::info(fmt::format(fmt::fg(color), args...));
}

std::vector<char> FetchCode(const std::string& filename);
std::string nfdOpen(nfdfilteritem_t filterItem);
std::string nfdSave(nfdu8filteritem_t filterItem);
std::string nfdPickFolder();

namespace vkn {
void CheckResult(vk::Result result, const char* file, int line);
}
#define CHECK_RESULT(result) CheckResult(result, __FILE__, __LINE__)

#endif