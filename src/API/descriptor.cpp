#include "descriptor.h"

void DescriptorManager::CreateSetLayout(const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(bindings.descriptorSetCount);

    for (int i = 0; i < bindings.descriptorSetCount; ++i) {

        vk::DescriptorSetLayoutBinding layoutBinding(bindings.indices[i], bindings.descriptorTypes[i], bindings.descriptorCounts[i], bindings.bindingStages[i]);
        layoutBindings.push_back(layoutBinding);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.descriptorSetCount, layoutBindings.data());

    descriptorSetLayout = Device::GetDevice().createDescriptorSetLayout(layoutInfo);
}

void DescriptorManager::CreatePool(uint32_t size, const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (int i = 0; i < bindings.descriptorSetCount; i++) {

        for (int j = 0; j < bindings.descriptorCounts[i]; j++) {

            vk::DescriptorPoolSize poolSize(bindings.descriptorTypes[i], size);
            poolSizes.push_back(poolSize);
        }
    }

    vk::DescriptorPoolCreateInfo poolInfo({}, size, static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

    descriptorPool = Device::GetDevice().createDescriptorPool(poolInfo);
}

void DescriptorManager::AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets)
{
    // TODO:: why 1?
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &descriptorSetLayout);

    descriptorSets = Device::GetDevice().allocateDescriptorSets(allocateInfo);
}

DescriptorManager::~DescriptorManager()
{
    Device::GetDevice().destroyDescriptorSetLayout(descriptorSetLayout);
}
