#include "buffer.h"

#include "context.h"

#include "vko/Assert.h"
#include "vko/Device.h"

namespace
{
    VkBufferUsageFlags get_usage(gfx::buffer_usage usage)
    {
        VkBufferUsageFlags bufferUsageFlags = 0;

        switch (usage)
        {
        case gfx::buffer_usage::vertex_index:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case gfx::buffer_usage::uniform:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        assert(false);
        return 0;
    }
}

gfx_vk::buffer::buffer(context& context, gfx::buffer_params const& params) : m_context(context)
{
    assert(!params.is_mutable); // TODO implement

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = params.size;
    bufferCreateInfo.usage = get_usage(params.usage);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VKO_VERIFY(vkCreateBuffer(context.get_device().getHandle(), &bufferCreateInfo, &m_allocator.getCallbacks(), &m_handle.get()));
}

gfx_vk::buffer::~buffer()
{
    if (!m_handle)
        return;

    vkDestroyBuffer(m_context.get_device().getHandle(), m_handle, &m_allocator.getCallbacks());
}

size_t gfx_vk::buffer::get_size() const
{
    assert(false);
    return 0;
}

void gfx_vk::buffer::upload_sync(nstl::blob_view bytes, size_t offset)
{
    assert(false);
}
