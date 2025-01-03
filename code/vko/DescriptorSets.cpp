#include "DescriptorSets.h"

#include "vko/Device.h"

vko::DescriptorSets::DescriptorSets(Device const& device, nstl::vector<VkDescriptorSet> handles)
    : m_device(&device)
    , m_handles(nstl::move(handles))
{

}

void vko::DescriptorSets::update(UpdateConfig const& updateConfig)
{
    nstl::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(updateConfig.buffers.size());

    nstl::vector<VkDescriptorImageInfo> imageInfos;
    imageInfos.reserve(updateConfig.images.size());

    nstl::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(bufferInfos.capacity() + imageInfos.capacity());

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
        assert(buffer.binding <= UINT32_MAX);
        descriptorWrite.dstBinding = static_cast<uint32_t>(buffer.binding);
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
    }

    for (UpdateConfig::SampledImage const& image : updateConfig.images)
    {
        assert(descriptorWrites.size() < descriptorWrites.capacity());
        VkWriteDescriptorSet& descriptorWrite = descriptorWrites.emplace_back();

        assert(imageInfos.size() < imageInfos.capacity());
        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = image.imageView;
        imageInfo.sampler = image.sampler;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_handles[image.set];
        assert(image.binding <= UINT32_MAX);
        descriptorWrite.dstBinding = static_cast<uint32_t>(image.binding);
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    vkUpdateDescriptorSets(m_device->getHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

size_t vko::DescriptorSets::getSize() const
{
    return m_handles.size();
}
