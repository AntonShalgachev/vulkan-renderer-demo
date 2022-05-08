#include "Mesh.h"

#include "wrapper/Buffer.h"
#include "wrapper/DeviceMemory.h"
#include "Utils.h"

vkr::Mesh const* vkr::Mesh::ms_boundMesh = nullptr;

vkr::Mesh::Mesh(Application const& app, std::vector<unsigned char> const& data, VertexLayout layout) : Object(app)
{
    m_vertexLayout = std::move(layout);
    createBuffers(data);
}

vkr::Mesh::~Mesh() = default;

void vkr::Mesh::bindBuffers(VkCommandBuffer commandBuffer) const
{
    if (ms_boundMesh == this)
        return;

    auto const& bindingOffsets = m_vertexLayout.getBindingOffsets();

    std::vector<VkBuffer> vertexBuffers;
    vertexBuffers.resize(bindingOffsets.size(), m_buffer->getHandle());

    vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(bindingOffsets.size()), vertexBuffers.data(), m_vertexLayout.getBindingOffsets().data());

    vkCmdBindIndexBuffer(commandBuffer, m_buffer->getHandle(), m_vertexLayout.getIndexDataOffset(), m_vertexLayout.getIndexType());

    ms_boundMesh = this;
}

void vkr::Mesh::createBuffers(std::vector<unsigned char> const& rawData)
{
    VkDeviceSize bufferSize = sizeof(rawData[0]) * rawData.size();
    void const* bufferData = rawData.data();

    std::unique_ptr<vkr::Buffer> stagingBuffer;
    std::unique_ptr<vkr::DeviceMemory> stagingBufferMemory;
    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    stagingBufferMemory->copyFrom(bufferData, bufferSize);

    utils::createBuffer(getApp(), bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);

    vkr::Buffer::copy(*stagingBuffer, *m_buffer);
}
