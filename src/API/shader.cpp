#include "shader.h"

vk::ShaderModule Shader::CreateModule(std::string filepath)
{
    std::vector<char> sourceCode = ReadCode(filepath);

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.setCodeSize(sourceCode.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(sourceCode.data()));

    return Device::GetHandle().device.createShaderModule(createInfo);
}

Shader::~Shader()
{
    Device::GetHandle().device.destroyShaderModule(vertexShaderModule);
    Device::GetHandle().device.destroyShaderModule(fragmentShaderModule);
}