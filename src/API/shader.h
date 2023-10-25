#ifndef _SHADER_H_
#define _SHADER_H_

#include "../common.h"
#include "device.h"

class Shader
{
public:
    Shader(const vk::Device& vkDevice) : vkDevice(vkDevice) {}
    ~Shader();
    vk::ShaderModule CreateModule(std::string filepath);

    vk::ShaderModule vertexShaderModule;
    vk::ShaderModule fragmentShaderModule;

private:
    const vk::Device& vkDevice;
};

#endif