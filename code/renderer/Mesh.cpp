#include "Mesh.h"

#include "wrapper/Buffer.h"
#include "wrapper/DeviceMemory.h"
#include "BufferWithMemory.h"

namespace
{
    vkr::BufferWithMemory createBuffer(vkr::Application const& app, std::vector<unsigned char> const& data)
    {
        VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
        void const* bufferData = data.data();

        vkr::BufferWithMemory stagingBuffer{ app, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        stagingBuffer.memory().copyFrom(bufferData, bufferSize);

        vkr::BufferWithMemory buffer{ app, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
        vkr::Buffer::copy(stagingBuffer.buffer(), buffer.buffer());

        return buffer;
    }
}

vkr::Mesh const* vkr::Mesh::ms_boundMesh = nullptr;

vkr::Mesh::Mesh(Application const& app, std::vector<unsigned char> const& data, VertexLayout layout)
    : m_buffer(createBuffer(app, data))
    , m_vertexLayout(std::move(layout))
{

}

vkr::Mesh::~Mesh() = default;

void vkr::Mesh::bindBuffers(VkCommandBuffer commandBuffer) const
{
    if (ms_boundMesh == this)
        return;

    auto const& bindingOffsets = m_vertexLayout.getBindingOffsets();

    VkBuffer bufferHandle = m_buffer.buffer().getHandle();

    std::vector<VkBuffer> vertexBuffers;
    vertexBuffers.resize(bindingOffsets.size(), bufferHandle);

    vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(bindingOffsets.size()), vertexBuffers.data(), m_vertexLayout.getBindingOffsets().data());

    vkCmdBindIndexBuffer(commandBuffer, bufferHandle, m_vertexLayout.getIndexDataOffset(), m_vertexLayout.getIndexType());

    ms_boundMesh = this;
}
