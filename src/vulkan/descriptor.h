// Wrapper class for Vulkan descriptors
// Vulkanのディスクリプタのラッパークラス

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "../common.h"
#include "device.h"

namespace vkn {
struct DescriptorBinding
{
    DescriptorBinding(vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags sFlags = {}, vk::DescriptorBindingFlags bFlags = {})
        : descriptorType(type), descriptorCount(count), stageFlags(sFlags), bindingFlags(bFlags) {}

    vk::DescriptorType descriptorType;
    uint32_t descriptorCount;
    vk::ShaderStageFlags stageFlags;
    vk::DescriptorBindingFlags bindingFlags;
};
class Descriptor
{
public:
    static vk::DescriptorSetLayout CreateDescriptorSetLayout(const std::vector<DescriptorBinding>& descriptorBindings, vk::DescriptorSetLayoutCreateFlags layoutCreateFlags = {});
    static void SetPoolSizes(std::vector<vk::DescriptorPoolSize>& poolSizes, const std::vector<DescriptorBinding>& descriptorBindings, uint32_t& maxSets);
    static void CreateDescriptorPool(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags = {});
    static void AllocateDescriptorSets(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorSet>& descriptorSets, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
};
} // namespace vkn

#endif