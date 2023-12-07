#include "shader.h"

vk::ShaderModule Shader::CreateModule(std::string filepath)
{
    std::vector<char> sourceCode = ReadFile(filepath);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(sourceCode.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(sourceCode.data()));

    return Device::GetDevice().createShaderModule(createInfo);
}

Shader::~Shader()
{
    Device::GetDevice().destroyShaderModule(vertexShaderModule);
    Device::GetDevice().destroyShaderModule(fragmentShaderModule);
}