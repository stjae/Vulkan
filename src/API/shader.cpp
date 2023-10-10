#include "shader.h"

vk::ShaderModule Shader::CreateModule(std::string filepath)
{
    std::vector<char> sourceCode = ReadFile(filepath);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(sourceCode.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(sourceCode.data()));

    return device.createShaderModule(createInfo);
}

Shader::~Shader()
{
    device.destroyShaderModule(vertexShaderModule);
    device.destroyShaderModule(fragmentShaderModule);
}