#include "descriptor.h"

void DescriptorManager::CreateSetLayout(const DescriptorSetLayoutData& bindings, const VkDescriptorSetLayoutBindingFlagsCreateInfo* const bindingFlags)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(bindings.descriptorSetCount);

    for (int i = 0; i < bindings.descriptorSetCount; ++i) {

        vk::DescriptorSetLayoutBinding layoutBinding(bindings.indices[i], bindings.descriptorTypes[i], bindings.descriptorCounts[i], bindings.bindingStages[i]);
        layoutBindings.push_back(layoutBinding);
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.descriptorSetCount, layoutBindings.data(), bindingFlags);

    descriptorSetLayouts.push_back(Device::GetDevice().createDescriptorSetLayout(layoutInfo));
}

void DescriptorManager::CreatePool(uint32_t size, const std::vector<DescriptorSetLayoutData>& descriptorSetLayouts)
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

    vk::DescriptorPoolCreateInfo poolInfo({}, size * descriptorSetLayouts.size(), static_cast<uint32_t>(poolSizes.size()), poolSizes.data());
    // #if defined(__APPLE__)
    //     // SRS - increase the per-stage descriptor samplers limit on macOS (maxPerStageDescriptorUpdateAfterBindSamplers > maxPerStageDescriptorSamplers)
    //     poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
    // #endif

    descriptorPool = Device::GetDevice().createDescriptorPool(poolInfo);
}

void DescriptorManager::AllocateSet(std::vector<vk::DescriptorSet>& descriptorSets)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, descriptorSetLayouts.size(), descriptorSetLayouts.data());

    descriptorSets = Device::GetDevice().allocateDescriptorSets(allocateInfo);
}

DescriptorManager::~DescriptorManager()
{
    for (auto& layout : descriptorSetLayouts)
        Device::GetDevice().destroyDescriptorSetLayout(layout);
}
