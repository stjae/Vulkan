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

class DescriptorManager
{
    friend class GraphicsPipeline;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    vk::DescriptorPool descriptorPool;

    void CreateSetLayout(const DescriptorSetLayoutData& bindings);
    void CreatePool(uint32_t size, const std::vector<DescriptorSetLayoutData>& descriptorSetLayouts, const vk::DescriptorPoolCreateFlags& poolCreateFlags);
    void AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets);
    ~DescriptorManager();

    const vk::DescriptorPool& GetDescriptorPool() { return descriptorPool; }
};

#endif