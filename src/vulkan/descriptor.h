#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "../common.h"
#include "device.h"

namespace vkn {
struct DescriptorBinding
{
    DescriptorBinding(uint32_t index, vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags sFlags = {}, vk::DescriptorBindingFlags bFlags = {})
        : bindingIndex(index), descriptorType(type), descriptorCount(count), stageFlags(sFlags), bindingFlags(bFlags) {}

    uint32_t bindingIndex;
    vk::DescriptorType descriptorType;
    uint32_t descriptorCount;
    vk::ShaderStageFlags stageFlags;
    vk::DescriptorBindingFlags bindingFlags;
};
class Descriptor
{
public:
    static vk::DescriptorSetLayout CreateDescriptorSetLayout(const std::__1::vector<DescriptorBinding>& descriptorBindings, vk::DescriptorSetLayoutCreateFlags layoutCreateFlags = {});
    static void SetPoolSizes(std::__1::vector<vk::DescriptorPoolSize>& poolSizes, const std::__1::vector<DescriptorBinding>& descriptorBindings, uint32_t& maxSets);
    static void CreateDescriptorPool(vk::DescriptorPool& descriptorPool, std::__1::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags = {});
    static void AllocateDescriptorSets(vk::DescriptorPool& descriptorPool, std::__1::vector<vk::DescriptorSet>& descriptorSets, const std::__1::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
};
} // namespace vkn

#endif