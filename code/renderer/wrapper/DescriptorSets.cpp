#include "DescriptorSets.h"
#include "Device.h"
#include <stdexcept>
#include <cassert>

vko::DescriptorSets::DescriptorSets(Device const& device, std::vector<VkDescriptorSet> handles)
    : m_device(&device)
    , m_handles(std::move(handles))
{

}

void vko::DescriptorSets::update(UpdateConfig const& updateConfig)
{
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(updateConfig.buffers.size());

    std::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.reserve(updateConfig.images.size());

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(bufferInfos.size() + imageInfos.size());

    for (UpdateConfig::Buffer const& buffer : updateConfig.buffers)
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        assert(bufferInfos.size() < bufferInfos.capacity());
        VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
        bufferInfo.buffer = buffer.buffer;
        bufferInfo.offset = buffer.offset;
        bufferInfo.range = buffer.size;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_handles[buffer.set];
        descriptorWrite.dstBinding = buffer.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
    }

    for (UpdateConfig::SampledImage const& image : updateConfig.images)
    {
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        assert(imageInfos.size() < imageInfos.capacity());
        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = image.imageView;
        imageInfo.sampler = image.sampler;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_handles[image.set];
        descriptorWrite.dstBinding = image.binding;
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
