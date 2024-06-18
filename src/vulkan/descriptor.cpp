#include "descriptor.h"
#include "device.h"

vk::DescriptorSetLayout vkn::Descriptor::CreateDescriptorSetLayout(const std::vector<vkn::DescriptorBinding>& descriptorBindings, vk::DescriptorSetLayoutCreateFlags layoutCreateFlags)
{
    std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
    std::vector<vk::DescriptorBindingFlags> bindingFlags;

    for (int i = 0; i < descriptorBindings.size(); i++) {
        layoutBindings.emplace_back(i, descriptorBindings[i].descriptorType, descriptorBindings[i].descriptorCount, descriptorBindings[i].stageFlags);
        if (descriptorBindings[i].bindingFlags)
            bindingFlags.emplace_back(descriptorBindings[i].bindingFlags);
    }
    vk::DescriptorSetLayoutBindingFlagsCreateInfo layoutBindingFlagsCreateInfo(bindingFlags.size(), bindingFlags.data());
    vk::DescriptorSetLayoutCreateInfo layoutCreateInfo(layoutCreateFlags, layoutBindings.size(), layoutBindings.data());
    layoutCreateInfo.pNext = bindingFlags.empty() ? nullptr : &layoutBindingFlagsCreateInfo;

    return vkn::Device::Get().device.createDescriptorSetLayout(layoutCreateInfo);
}

void vkn::Descriptor::SetPoolSizes(std::vector<vk::DescriptorPoolSize>& poolSizes, const std::vector<vkn::DescriptorBinding>& descriptorBindings, uint32_t& maxSets)
{
    for (const auto& binding : descriptorBindings) {
        poolSizes.emplace_back(binding.descriptorType, binding.descriptorCount);
        maxSets += binding.descriptorCount;
    }
}

void vkn::Descriptor::CreateDescriptorPool(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorPoolSize>& poolSizes, uint32_t maxSets, const vk::DescriptorPoolCreateFlags& descriptorPoolCreateFlags)
{
    vk::DescriptorPoolCreateInfo poolInfo(descriptorPoolCreateFlags, maxSets, uint32_t(poolSizes.size()), poolSizes.data());
    descriptorPool = vkn::Device::Get().device.createDescriptorPool(poolInfo);
}

void vkn::Descriptor::AllocateDescriptorSets(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorSet>& descriptorSets, const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts)
{
    vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, descriptorSetLayouts.size(), descriptorSetLayouts.data());
    descriptorSets = vkn::Device::Get().device.allocateDescriptorSets(allocateInfo);
}