#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

// glfw, vulkan library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_win32.h>

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

#include "config.h"

std::vector<char> ReadFile(const std::string& filename);

// spdlog::info
template <typename... T>
void Log(bool debug, fmt::terminal_color color, T... args)
{
    if (!debug) {
        return;
    }
    spdlog::info(fmt::format(fmt::fg(color), args...));
}

#endif