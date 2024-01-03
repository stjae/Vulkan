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
    vk::DescriptorPool descriptorPool_;
    std::vector<vk::DescriptorSet> descriptorSets_;

    static vk::DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayoutData& bindings);
    void CreateDescriptorPool(uint32_t frameCount, const std::vector<DescriptorSetLayoutData>& descriptorSetLayoutData, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags);
    void AllocateDescriptorSets(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts);
    ~Descriptor();
};

#endif