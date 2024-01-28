#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include "../common.h"
#include "device.h"

struct DescriptorSetLayoutData
{
    int descriptorSetCount = 0;
    std::vector<int> indices;
    std::vector<vk::DescriptorType> descriptorTypes;
    std::vector<int> descriptorCounts;
    std::vector<vk::ShaderStageFlags> bindingStages;
    std::vector<vk::DescriptorBindingFlags> bindingFlags;
    vk::DescriptorSetLayoutCreateFlags layoutCreateFlags;
};

class Descriptor
{
public:
    static vk::DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayoutData& bindings);
    static void CreateDescriptorPool(vk::DescriptorPool& descriptorPool, uint32_t descriptorCount, const std::vector<DescriptorSetLayoutData>& descriptorSetLayoutData, uint32_t descriptorSetLayoutCount, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags);
    static void AllocateDescriptorSets(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorSet>& descriptorSets, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    //    ~Descriptor();
};

#endif