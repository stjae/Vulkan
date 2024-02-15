#include "descriptor.h"

vk::DescriptorSetLayout Descriptor::CreateDescriptorSetLayout(const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    std::vector<vk::DescriptorSetLayoutBindingFlagsCreateInfo> layoutBindingFlags;
    layoutBindings.reserve(bindings.bindingCount);

    for (int i = 0; i < bindings.bindingCount; ++i) {

        layoutBindings.emplace_back(bindings.indices[i], bindings.descriptorTypes[i], bindings.descriptorSetCount[i], bindings.bindingStages[i]);
        layoutBindingFlags.emplace_back(bindings.bindingCount, &bindings.bindingFlags[i]);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo(bindings.layoutCreateFlags, bindings.bindingCount, layoutBindings.data(), layoutBindingFlags.data());

    return Device::GetBundle().device.createDescriptorSetLayout(layoutInfo);
}

void Descriptor::CreateDescriptorPool(vk::DescriptorPool& descriptorPool, uint32_t descriptorCount, const std::vector<DescriptorSetLayoutData>& descriptorSetLayoutData, uint32_t descriptorSetLayoutCount, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (auto& layout : descriptorSetLayoutData) {
        for (int i = 0; i < layout.bindingCount; i++) {
            for (int j = 0; j < layout.descriptorSetCount[i]; j++) {
                // per descriptor set
                vk::DescriptorPoolSize poolSize(layout.descriptorTypes[i], descriptorCount);
                poolSizes.push_back(poolSize);
            }
        }
    }

    vk::DescriptorPoolCreateInfo poolInfo(descriptorPoolCreateFlags, descriptorCount * descriptorSetLayoutCount, static_cast<uint32_t>(poolSizes.size()), poolSizes.data());
    descriptorPool = Device::GetBundle().device.createDescriptorPool(poolInfo);
}

void Descriptor::AllocateDescriptorSets(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorSet>& descriptorSets, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, descriptorSetLayouts.size(), descriptorSetLayouts.data());
    descriptorSets = Device::GetBundle().device.allocateDescriptorSets(allocateInfo);
}