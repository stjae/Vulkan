#include "descriptor.h"

vk::DescriptorSetLayout Descriptor::CreateDescriptorSetLayout(const DescriptorSetLayoutData& bindings)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    std::vector<vk::DescriptorSetLayoutBindingFlagsCreateInfo> layoutBindingFlags;
    layoutBindings.reserve(bindings.descriptorSetCount);

    for (int i = 0; i < bindings.descriptorSetCount; ++i) {

        layoutBindings.emplace_back(bindings.indices[i], bindings.descriptorTypes[i], bindings.descriptorCounts[i], bindings.bindingStages[i]);
        layoutBindingFlags.emplace_back(bindings.bindingFlags[i]);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo(bindings.layoutCreateFlags, bindings.descriptorSetCount, layoutBindings.data(), layoutBindingFlags.data());

    return Device::GetHandle().device.createDescriptorSetLayout(layoutInfo);
}

void Descriptor::CreateDescriptorPool(uint32_t frameCount, const std::vector<DescriptorSetLayoutData>& descriptorSetLayoutData, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for (auto& layoutData : descriptorSetLayoutData) {
        for (int i = 0; i < layoutData.descriptorSetCount; i++) {

            for (int j = 0; j < layoutData.descriptorCounts[i]; j++) {

                vk::DescriptorPoolSize poolSize(layoutData.descriptorTypes[i], frameCount);
                poolSizes.push_back(poolSize);
            }
        }
    }

    vk::DescriptorPoolCreateInfo poolInfo(descriptorPoolCreateFlags, frameCount * descriptorSetLayouts.size(), static_cast<uint32_t>(poolSizes.size()), poolSizes.data());
    descriptorPool_ = Device::GetHandle().device.createDescriptorPool(poolInfo);
}

void Descriptor::AllocateDescriptorSets(const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool_, descriptorSetLayouts.size(), descriptorSetLayouts.data());
    descriptorSets_ = Device::GetHandle().device.allocateDescriptorSets(allocateInfo);
}

Descriptor::~Descriptor()
{
    Device::GetHandle().device.destroyDescriptorPool(descriptorPool_);
}
