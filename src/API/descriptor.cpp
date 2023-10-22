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

    m_setLayout = device.createDescriptorSetLayout(layoutInfo);
}

void Descriptor::CreatePool(uint32_t size, const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (int i = 0; i < bindings.count; ++i) {

        vk::DescriptorPoolSize poolSize(bindings.types[i], size);
        poolSizes.push_back(poolSize);
    }

    vk::DescriptorPoolCreateInfo poolInfo({}, size, static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

    m_pool = device.createDescriptorPool(poolInfo);
}

vk::DescriptorSet Descriptor::AllocateSet(vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout layout)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &layout);

    return device.allocateDescriptorSets(allocateInfo)[0];
}

Descriptor::~Descriptor()
{
    device.destroyDescriptorSetLayout(m_setLayout);
}
