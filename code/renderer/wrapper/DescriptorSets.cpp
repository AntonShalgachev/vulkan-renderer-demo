#include "DescriptorSets.h"
#include "DescriptorPool.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Device.h"
#include <array>
#include <list>
#include <stdexcept>
#include "ImageView.h"

vko::DescriptorSets::DescriptorSets(Device const& device, std::vector<VkDescriptorSet> handles)
    : m_device(&device)
    , m_handles(std::move(handles))
{

}

void vko::DescriptorSets::update(std::size_t index, Buffer const& uniformBuffer, std::size_t bufferSize, vkr::Texture const* texture, vkr::Texture const* normalMap)
{
    VkDescriptorSet setHandle = m_handles[index];

    std::vector<VkWriteDescriptorSet> descriptorWrites;

    // to keep these objects around during a Vulkan call and to avoid invalidation
    std::list<VkDescriptorBufferInfo> bufferInfos;
    std::list<VkDescriptorImageInfo> imageInfos;

    // TODO couple it with the data within DescriptorPool
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
        bufferInfo.buffer = uniformBuffer.getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = bufferSize;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = setHandle;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
    }

    if (texture)
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->getImageView().getHandle();
        imageInfo.sampler = texture->getSampler().getHandle();

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = setHandle;
        descriptorWrite.dstBinding = 1;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    if (normalMap)
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = normalMap->getImageView().getHandle();
        imageInfo.sampler = normalMap->getSampler().getHandle();

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = setHandle;
        descriptorWrite.dstBinding = 2;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    vkUpdateDescriptorSets(m_device->getHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

std::size_t vko::DescriptorSets::getSize() const
{
    return m_handles.size();
}
