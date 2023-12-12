#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include "../common.h"
#include "device.h"

struct DescriptorSetLayoutData
{
    int descriptorSetCount = 0;
    std::vector<int> indices;
    std::vector<vk::DescriptorType> descriptorTypes;
    std::vector<int> descriptorCounts;
    std::vector<vk::ShaderStageFlags> bindingStages;
};

class DescriptorManager
{
    friend class GraphicsPipeline;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
    vk::DescriptorPool descriptorPool;

    void CreateSetLayout(const DescriptorSetLayoutData& bindings);
    void CreatePool(uint32_t size, const std::vector<DescriptorSetLayoutData>& descriptorSetLayouts);
    void AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets);
    ~DescriptorManager();

    const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayout() { return descriptorSetLayouts; }
    const vk::DescriptorPool& GetDescriptorPool() { return descriptorPool; }
};

#endif