#include "descriptor.h"

void DescriptorManager::CreateSetLayout(const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    std::vector<vk::DescriptorSetLayoutBindingFlagsCreateInfo> layoutBindingFlags;
    layoutBindings.reserve(bindings.descriptorSetCount);

    for (int i = 0; i < bindings.descriptorSetCount; ++i) {

        layoutBindings.emplace_back(bindings.indices[i], bindings.descriptorTypes[i], bindings.descriptorCounts[i], bindings.bindingStages[i]);
        layoutBindingFlags.emplace_back(bindings.bindingFlags[i]);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo(bindings.layoutCreateFlags, bindings.descriptorSetCount, layoutBindings.data(), layoutBindingFlags.data());

    descriptorSetLayouts.push_back(Device::GetHandle().device.createDescriptorSetLayout(layoutInfo));
}

void DescriptorManager::CreatePool(uint32_t size, const std::vector<DescriptorSetLayoutData>& descriptorSetLayouts, const vk::DescriptorPoolCreateFlags& poolCreateFlags)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (auto& layout : descriptorSetLayouts) {
        for (int i = 0; i < layout.descriptorSetCount; i++) {

            for (int j = 0; j < layout.descriptorCounts[i]; j++) {

                vk::DescriptorPoolSize poolSize(layout.descriptorTypes[i], size);
                poolSizes.push_back(poolSize);
            }
        }
    }

    vk::DescriptorPoolCreateInfo poolInfo(poolCreateFlags, size * descriptorSetLayouts.size(), static_cast<uint32_t>(poolSizes.size()), poolSizes.data());

    descriptorPool = Device::GetHandle().device.createDescriptorPool(poolInfo);
}

void DescriptorManager::AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, descriptorSetLayouts.size(), descriptorSetLayouts.data());

    descriptorSets = Device::GetHandle().device.allocateDescriptorSets(allocateInfo);
}

DescriptorManager::~DescriptorManager()
{
    for (auto& layout : descriptorSetLayouts)
        Device::GetHandle().device.destroyDescriptorSetLayout(layout);
}
