#include "descriptor.h"

void Descriptor::CreateSetLayout(const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(bindings.count);

    for (int i = 0; i < bindings.count; ++i) {

        vk::DescriptorSetLayoutBinding layoutBinding(bindings.indices[i], bindings.types[i], bindings.counts[i], bindings.stages[i]);
        layoutBindings.push_back(layoutBinding);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.count, layoutBindings.data());

    setLayout = vkDevice.createDescriptorSetLayout(layoutInfo);
}

void Descriptor::CreatePool(uint32_t size, const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (int i = 0; i < bindings.count; i++) {
        for (int j = 0; j < bindings.counts[i]; j++) {

            vk::DescriptorPoolSize poolSize(bindings.types[i], size);
            poolSizes.push_back(poolSize);
        }
    }

    vk::DescriptorPoolCreateInfo poolInfo({}, size, static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

    pool = vkDevice.createDescriptorPool(poolInfo);
}

void Descriptor::AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets)
{
    vk::DescriptorSetAllocateInfo allocateInfo(pool, 1, &setLayout);

    descriptorSets = vkDevice.allocateDescriptorSets(allocateInfo);
}

Descriptor::~Descriptor()
{
    vkDevice.destroyDescriptorSetLayout(setLayout);
}
