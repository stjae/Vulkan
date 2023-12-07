#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include "../common.h"
#include "device.h"

struct DescriptorSetLayoutData
{
    int count;
    std::vector<int> indices;
    std::vector<vk::DescriptorType> types;
    std::vector<int> counts;
    std::vector<vk::ShaderStageFlags> stages;
};

class Descriptor
{
    friend class GraphicsPipeline;

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorPool descriptorPool;

    void CreateSetLayout(const DescriptorSetLayoutData& bindings);
    void CreatePool(uint32_t size, const DescriptorSetLayoutData& bindings);
    void AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets);
    ~Descriptor();

    const vk::DescriptorSetLayout& GetDescriptorSetLayout() { return descriptorSetLayout; }
    const vk::DescriptorPool& GetDescriptorPool() { return descriptorPool; }
};

#endif