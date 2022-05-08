#include "Mesh.h"

#include "wrapper/Buffer.h"

vkr::Mesh const* vkr::Mesh::ms_boundMesh = nullptr;

vkr::Mesh::Mesh(Application const& app, Buffer const& buffer, VertexLayout layout)
    : m_buffer(buffer)
    , m_vertexLayout(std::move(layout))
{

}

vkr::Mesh::~Mesh() = default;

void vkr::Mesh::bindBuffers(VkCommandBuffer commandBuffer) const
{
    if (ms_boundMesh == this)
        return;

    auto const& bindingOffsets = m_vertexLayout.getBindingOffsets();

    VkBuffer bufferHandle = m_buffer.getHandle();

    std::vector<VkBuffer> vertexBuffers;
    vertexBuffers.resize(bindingOffsets.size(), bufferHandle);

    vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(bindingOffsets.size()), vertexBuffers.data(), m_vertexLayout.getBindingOffsets().data());

    vkCmdBindIndexBuffer(commandBuffer, bufferHandle, m_vertexLayout.getIndexDataOffset(), m_vertexLayout.getIndexType());

    ms_boundMesh = this;
}
