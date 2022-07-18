#include "DescriptorSets.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Device.h"
#include <array>
#include <stdexcept>

vkr::DescriptorSets::DescriptorSets(Application const& app, DescriptorPool const& pool, DescriptorSetLayout const& layout)
    : Object(app)
    , m_pool(pool)
    , m_layout(layout)
{
    std::vector<VkDescriptorSetLayout> layouts(pool.getSize(), layout.getHandle());

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_pool.getHandle();
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(pool.getSize());
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    m_handles.resize(pool.getSize());
    if (vkAllocateDescriptorSets(getDevice().getHandle(), &descriptorSetAllocInfo, m_handles.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");
}

vkr::DescriptorSets::~DescriptorSets()
{
    // no need to explicitly free descriptor sets
}

void vkr::DescriptorSets::update(std::size_t index, Buffer const& uniformBuffer, Texture const* texture, Sampler const* sampler)
{
    VkDescriptorSet setHandle = m_handles[index];

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // to keep these objects around during a Vulkan call
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;

    // TODO couple it with the data within DescriptorPool
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
        bufferInfo.buffer = uniformBuffer.getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = uniformBuffer.getSize();

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = setHandle;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
    }

    bool hasSampler = texture && sampler;
    if (hasSampler != m_layout.hasSampler())
        throw std::runtime_error("Invalid descriptor set layout");

    if (texture && sampler)
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->getImageViewHandle();
        imageInfo.sampler = sampler->getHandle();

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = setHandle;
        descriptorWrite.dstBinding = 1;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    vkUpdateDescriptorSets(getDevice().getHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::size_t vkr::DescriptorSets::getSize() const
{
    return m_pool.getSize();
}
