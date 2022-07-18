#include "Mesh.h"

#include "wrapper/Buffer.h"

vkr::Mesh::Mesh(Application const& app, Buffer const& buffer, VertexLayout layout, Metadata metadata)
    : m_buffer(buffer)
    , m_vertexLayout(std::move(layout))
    , m_metadata(std::move(metadata))
{

}

vkr::Mesh::~Mesh() = default;

void vkr::Mesh::draw(VkCommandBuffer commandBuffer) const
{
	auto const& bindingOffsets = m_vertexLayout.getBindingOffsets();

	VkBuffer bufferHandle = m_buffer.getHandle();

	std::vector<VkBuffer> vertexBuffers;
	vertexBuffers.resize(bindingOffsets.size(), bufferHandle);

	vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(bindingOffsets.size()), vertexBuffers.data(), bindingOffsets.data());
	vkCmdBindIndexBuffer(commandBuffer, bufferHandle, m_vertexLayout.getIndexDataOffset(), m_vertexLayout.getIndexType());

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_vertexLayout.getIndexCount()), 1, 0, 0, 0);
}
