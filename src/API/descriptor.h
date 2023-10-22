#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include "../common.h"
#include "device.h"

struct DescriptorSetLayoutData {

    int count;
    std::vector<int> indices;
    std::vector<vk::DescriptorType> types;
    std::vector<int> counts;
    std::vector<vk::ShaderStageFlags> stages;
};

inline vk::DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(bindings.count);

    for (int i = 0; i < bindings.count; ++i) {

        vk::DescriptorSetLayoutBinding layoutBinding(bindings.indices[i], bindings.types[i], bindings.counts[i], bindings.stages[i]);
        layoutBindings.push_back(layoutBinding);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.count, layoutBindings.data());

    return device.createDescriptorSetLayout(layoutInfo);
}

inline vk::DescriptorPool CreateDescriptorPool(uint32_t size, const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (int i = 0; i < bindings.count; ++i) {

        vk::DescriptorPoolSize poolSize(bindings.types[i], size);
        poolSizes.push_back(poolSize);
    }

    vk::DescriptorPoolCreateInfo poolInfo({}, size, static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

    return device.createDescriptorPool(poolInfo);
}

inline vk::DescriptorSet AllocateDescriptorSet(vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout layout)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &layout);

    return device.allocateDescriptorSets(allocateInfo)[0];
}

inline vk::DescriptorSetLayout descriptorSetLayout;
inline vk::DescriptorPool descriptorPool;

#endif