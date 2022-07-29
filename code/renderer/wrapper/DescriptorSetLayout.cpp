#include "DescriptorSetLayout.h"
#include "Device.h"
#include <array>
#include <stdexcept>

vkr::DescriptorSetLayout::DescriptorSetLayout(Application const& app, DescriptorSetConfiguration const& config) : Object(app), m_configuration(std::move(config))
{
    // TODO pass configuration externally

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    VkDescriptorSetLayoutBinding& uboLayoutBinding = bindings.emplace_back();
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT; // TODO specify dynamically
    uboLayoutBinding.pImmutableSamplers = nullptr;

    if (m_configuration.hasTexture)
    {
        VkDescriptorSetLayoutBinding& samplerLayoutBinding = bindings.emplace_back();
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
    }

    if (m_configuration.hasNormalMap)
    {
        VkDescriptorSetLayoutBinding& samplerLayoutBinding = bindings.emplace_back();
        samplerLayoutBinding.binding = 2;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(getDevice().getHandle(), &descriptorSetLayoutCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
}

vkr::DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(getDevice().getHandle(), m_handle, nullptr);
}
