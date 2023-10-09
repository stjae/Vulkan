#ifndef _SHADER_H_
#define _SHADER_H_

#include "../common.h"
#include "device.h"

class Shader
{
public:
    ~Shader();
    vk::ShaderModule CreateModule(std::string filepath);

    static vk::ShaderModule vertexShaderModule;
    static vk::ShaderModule fragmentShaderModule;
};

#endif