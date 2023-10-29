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
    Descriptor(const vk::Device& vkDevice) : vkDevice(vkDevice) {}
    void CreateSetLayout(const DescriptorSetLayoutData& bindings);
    void CreatePool(uint32_t size, const DescriptorSetLayoutData& bindings);
    void AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets);
    ~Descriptor();

    // vk::DescriptorSet descriptorSet;
    vk::DescriptorSetLayout setLayout;
    vk::DescriptorPool pool;

private:
    const vk::Device& vkDevice;
};

#endif