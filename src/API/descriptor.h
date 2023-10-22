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

class Descriptor
{
public:
    void CreateSetLayout(const DescriptorSetLayoutData& bindings);
    void CreatePool(uint32_t size, const DescriptorSetLayoutData& bindings);
    vk::DescriptorSet AllocateSet(vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout layout);
    ~Descriptor();

    vk::DescriptorSetLayout m_setLayout;
    vk::DescriptorPool m_pool;
};

#endif