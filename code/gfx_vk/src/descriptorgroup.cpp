#include "descriptorgroup.h"

#include "context.h"

#include "buffer.h"
#include "image.h"
#include "sampler.h"

#include "nstl/static_vector.h"

namespace
{
    struct temp_resources
    {
        // TODO remove this hack. static_vector is only needed to prevent reallocation
        nstl::static_vector<VkDescriptorBufferInfo, 6> buffers;
        nstl::static_vector<VkDescriptorImageInfo, 6> images;
    };

    void add_buffer_write(gfx_vk::context& context, temp_resources& resources, VkWriteDescriptorSet& write, gfx::buffer_handle buffer, size_t subresource_index)
    {
        gfx_vk::buffer const& resource = context.get_resources().get_buffer(buffer);

        size_t index = resource.is_mutable() ? subresource_index : 0;

        resources.buffers.push_back({
            .buffer = resource.get_handle(index),
            .offset = 0,
            .range = resource.get_size(),
        });

        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pBufferInfo = &resources.buffers.back();
    }

    void add_combined_image_sampler_write(gfx_vk::context& context, temp_resources& resources, VkWriteDescriptorSet& write, gfx::image_handle image, gfx::sampler_handle sampler)
    {
        resources.images.push_back({
            .sampler = context.get_resources().get_sampler(sampler).get_handle(),
            .imageView = context.get_resources().get_image(image).get_view_handle(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        });

        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &resources.images.back();
    }
}

gfx_vk::descriptorgroup::descriptorgroup(context& context, gfx::descriptorgroup_params const& params)
    : m_context(context)
{
    nstl::vector<gfx::descriptor_layout_entry> entries;
    entries.reserve(params.entries.size());
    m_is_mutable = false;

    for (gfx::descriptorgroup_entry const& entry : params.entries)
    {
        entries.push_back({
            .location = entry.location,
            .type = entry.resource.type,
        });

        bool is_resource_mutable = false;
        switch (entry.resource.type)
        {
        case gfx::descriptor_type::uniform_buffer:
            is_resource_mutable = context.get_resources().get_buffer(entry.resource.buffer).is_mutable();
            break;
        case gfx::descriptor_type::combined_image_sampler:
            is_resource_mutable = false; // TODO implement
            break;
        }

        if (is_resource_mutable)
            m_is_mutable = true;
    }

    size_t sets_count = m_is_mutable ? m_context.get_mutable_resource_multiplier() : 1;

    gfx::descriptorgroup_layout_view layout = { .entries = entries };
    VkDescriptorSetLayout vk_layout = m_context.get_resources().create_descriptor_set_layout(layout);

    nstl::vector<VkDescriptorSetLayout> vk_layouts;
    for (size_t i = 0; i < sets_count; i++)
        vk_layouts.push_back(vk_layout);

    m_handles.resize(sets_count);

    m_context.get_descriptor_allocator().allocate(vk_layouts, m_handles);

    nstl::vector<VkWriteDescriptorSet> writes;
    temp_resources resources;

    for (size_t i = 0; i < sets_count; i++)
    {
        for (gfx::descriptorgroup_entry const& entry : params.entries)
        {
            assert(entry.location <= UINT32_MAX);

            writes.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_handles[i],
                .dstBinding = static_cast<uint32_t>(entry.location),
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM,
            });

            switch (entry.resource.type)
            {
            case gfx::descriptor_type::uniform_buffer:
                add_buffer_write(m_context, resources, writes.back(), entry.resource.buffer, i);
                break;
            case gfx::descriptor_type::combined_image_sampler:
                add_combined_image_sampler_write(m_context, resources, writes.back(), entry.resource.combined_image_sampler.image, entry.resource.combined_image_sampler.sampler);
                break;
            }

            assert(writes.back().descriptorType != VK_DESCRIPTOR_TYPE_MAX_ENUM);
        }
    }

    vkUpdateDescriptorSets(m_context.get_device().getHandle(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

gfx_vk::descriptorgroup::~descriptorgroup()
{
    // TODO free descriptors + destroy descriptor set layout
}

VkDescriptorSet gfx_vk::descriptorgroup::get_current_handle() const
{
    size_t index = m_is_mutable ? m_context.get_mutable_resource_index() : 0;
    return m_handles[index];
}
