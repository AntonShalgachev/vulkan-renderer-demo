#include "DescriptorSetLayout.h"

#include "vko/Device.h"
#include "vko/Assert.h"

vko::DescriptorSetLayout::DescriptorSetLayout(Device const& device, DescriptorSetConfiguration config)
    : m_device(device)
    , m_configuration(nstl::move(config))
{
    // TODO pass configuration externally

    nstl::vector<VkDescriptorSetLayoutBinding> bindings;

    if (m_configuration.hasBuffer)
    {
        VkDescriptorSetLayoutBinding& uboLayoutBinding = bindings.emplace_back();
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT; // TODO specify dynamically
        uboLayoutBinding.pImmutableSamplers = nullptr;
    }

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

    if (m_configuration.hasShadowMap)
    {
        VkDescriptorSetLayoutBinding& samplerLayoutBinding = bindings.emplace_back();
        samplerLayoutBinding.binding = 3;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();

    VKO_VERIFY(vkCreateDescriptorSetLayout(m_device.getHandle(), &descriptorSetLayoutCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

vko::DescriptorSetLayout::~DescriptorSetLayout()
{
    if (m_handle)
        vkDestroyDescriptorSetLayout(m_device.getHandle(), m_handle, &m_allocator.getCallbacks());
}
