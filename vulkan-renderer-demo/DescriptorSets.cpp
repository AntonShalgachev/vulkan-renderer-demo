#include "DescriptorSets.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"

vkr::DescriptorSets::DescriptorSets(std::size_t size, DescriptorSetLayout const& layout)
    : m_pool(std::make_unique<vkr::DescriptorPool>(size))
{
    std::vector<VkDescriptorSetLayout> layouts(size, layout.getHandle());

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_pool->getHandle();
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    m_handles.resize(size);
    if (vkAllocateDescriptorSets(temp::getDevice(), &descriptorSetAllocInfo, m_handles.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");
}

vkr::DescriptorSets::~DescriptorSets()
{
    // no need to explicitly free descriptor sets
}

void vkr::DescriptorSets::update(std::size_t index, Buffer const& uniformBuffer, Texture const& texture, Sampler const& sampler)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer.getHandle();
    bufferInfo.offset = 0;
    bufferInfo.range = uniformBuffer.getSize();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.getImageViewHandle();
    imageInfo.sampler = sampler.getHandle();

    VkDescriptorSet setHandle = m_handles[index];

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    // TODO couple it with the data within DescriptorPool
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = setHandle;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = setHandle;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(vkr::temp::getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::size_t vkr::DescriptorSets::getSize() const
{
    return m_pool->getSize();
}
