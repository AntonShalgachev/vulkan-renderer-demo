#include "ResourceManager.h"

#include "vko/CommandBuffers.h"
#include "vko/CommandPool.h"
#include "vko/Queue.h"
#include "vko/ShaderModule.h"
#include "vko/DescriptorSetLayout.h"
#include "vko/PipelineLayout.h"
#include "vko/Pipeline.h"
#include "vko/PhysicalDevice.h"
#include "vko/Sampler.h"

#include "vkgfx/Image.h"
#include "vkgfx/Buffer.h"
#include "vkgfx/Handles.h"
#include "vkgfx/Texture.h"
#include "vkgfx/Material.h"
#include "vkgfx/Mesh.h"
#include "vkgfx/PipelineKey.h"

#include "memory/tracking.h"

#include "nstl/optional.h"
#include "nstl/algorithm.h"

#include <assert.h>

namespace
{
    auto scopeId = memory::tracking::create_scope_id("Rendering/Resources");

    class OneTimeCommandBuffer
    {
    public:
        OneTimeCommandBuffer(vko::CommandPool const& commandPool, vko::Queue const& queue)
            : m_queue(queue)
            , m_buffers(commandPool.allocate(1))
        {
            m_buffers.begin(0, true);
        }

        VkCommandBuffer getHandle()
        {
            return m_buffers.getHandle(0);
        }

        void submit()
        {
            m_buffers.end(0);

            m_buffers.submit(0, m_queue, nullptr, nullptr, nullptr);
            m_queue.waitIdle();
        }

    private:
        vko::Queue const& m_queue;

        vko::CommandBuffers m_buffers;
    };

    // TODO get rid of it
    class BufferWithMemory
    {
    public:
        BufferWithMemory(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
            : m_buffer(vko::Buffer{ device, size, usage })
            , m_memory(vko::DeviceMemory{ device, physicalDevice, m_buffer->getMemoryRequirements(), properties })
        {
            m_buffer->bindMemory(*m_memory);
        }

        ~BufferWithMemory()
        {
            // TODO remove this nasty hack
            m_buffer = {};
            m_memory = {};
        }

        BufferWithMemory(BufferWithMemory&& rhs) = default;
        BufferWithMemory& operator=(BufferWithMemory&& rhs) = default;

        vko::Buffer const& buffer() const { return *m_buffer; }
        vko::DeviceMemory const& memory() const { return *m_memory; }

    private:
        // TODO remove this nasty hack
        nstl::optional<vko::Buffer> m_buffer;
        nstl::optional<vko::DeviceMemory> m_memory;
    };

    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            assert(false);
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }

    VkFormat vulkanizeAttributeFormat(vkgfx::AttributeType type)
    {
        switch (type)
        {
        case vkgfx::AttributeType::Vec2f:
            return VK_FORMAT_R32G32_SFLOAT;
        case vkgfx::AttributeType::Vec3f:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case vkgfx::AttributeType::Vec4f:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case vkgfx::AttributeType::UInt32:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            assert(false);
        }

        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }

    size_t alignSize(size_t originalSize, size_t alignment)
    {
        if (alignment == 0)
            return originalSize;

        return (originalSize + alignment - 1) & ~(alignment - 1);
    }
}

vkgfx::ResourceManager::ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::Queue const& uploadQueue, vko::RenderPass const& renderPass, vko::RenderPass const& shadowmapRenderPass, size_t resourceCount)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_uploadQueue(uploadQueue)
    , m_renderPass(renderPass)
    , m_shadowmapRenderPass(shadowmapRenderPass)
    , m_resourceCount(resourceCount)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_uploadCommandPool = nstl::make_unique<vko::CommandPool>(device, uploadQueue.getFamily()); // TODO set debug name for it
}

vkgfx::ResourceManager::~ResourceManager() = default;

void vkgfx::ResourceManager::setSubresourceIndex(size_t index)
{
    m_currentSubresourceIndex = index;
}

vkgfx::ImageHandle vkgfx::ResourceManager::createImage(ImageMetadata metadata)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(metadata.width > 0);
    assert(metadata.height > 0);
    assert(metadata.byteSize > 0);

    VkFormat vkFormat = [](ImageFormat format) {
        switch (format)
        {
        case ImageFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        case ImageFormat::BC1_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case ImageFormat::BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case ImageFormat::BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        }

        assert(false);
        return VK_FORMAT_R8G8B8A8_UNORM;
    }(metadata.format);

    auto width = static_cast<uint32_t>(metadata.width);
    auto height = static_cast<uint32_t>(metadata.height);

    vko::Image vkImage{ m_device, width, height, vkFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
    vko::DeviceMemory vkImageMemory{ m_device, m_physicalDevice, vkImage.getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
    vkImage.bindMemory(vkImageMemory);

    vko::ImageView vkImageView{ m_device, vkImage, VK_IMAGE_ASPECT_COLOR_BIT };

    Image imageResource = {
        .memory = nstl::move(vkImageMemory),
        .image = nstl::move(vkImage),
        .imageView = nstl::move(vkImageView),
        .metadata = nstl::move(metadata),
    };

    return { m_images.add(nstl::move(imageResource)) };
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, void const* data, size_t dataSize)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(handle);
    Image const* image = getImage(handle);
    assert(image);

    return uploadImage(*image, data, dataSize);
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, nstl::blob_view bytes)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return uploadImage(handle, bytes.data(), bytes.size());
}

vkgfx::Image* vkgfx::ResourceManager::getImage(ImageHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_images.get(handle);
}

void vkgfx::ResourceManager::removeImage(ImageHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_images.remove(handle);
}

vkgfx::Image const* vkgfx::ResourceManager::getImage(ImageHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_images.get(handle);
}

void vkgfx::ResourceManager::reserveMoreBuffers(size_t size)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_buffers.reserve(m_buffers.size() + size);
}

vkgfx::BufferHandle vkgfx::ResourceManager::createBuffer(size_t size, BufferMetadata metadata)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    VkBufferUsageFlags bufferUsageFlags = 0;
    VkMemoryPropertyFlags memoryPropertiesFlags = 0;

    switch (metadata.usage)
    {
    case BufferUsage::VertexIndexBuffer:
        bufferUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case BufferUsage::UniformBuffer:
        bufferUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    }

    switch (metadata.location)
    {
    case BufferLocation::DeviceLocal:
        memoryPropertiesFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bufferUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        break;
    case BufferLocation::HostVisible:
        memoryPropertiesFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    }

    size_t alignedSize = alignSize(size, m_physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment);

    nstl::vector<vko::Buffer> buffers;

    size_t subresourceCount = metadata.isMutable ? m_resourceCount : 1;
    size_t totalBufferSize = alignedSize * subresourceCount;

    buffers.reserve(subresourceCount);

    VkMemoryRequirements memoryRequirements{};
    for (size_t i = 0; i < subresourceCount; i++)
    {
        buffers.emplace_back(m_device, alignedSize, bufferUsageFlags);

        VkMemoryRequirements subresourceMemoryRequirements = buffers.back().getMemoryRequirements();

        memoryRequirements.size += subresourceMemoryRequirements.size;
        memoryRequirements.alignment = nstl::max(memoryRequirements.alignment, subresourceMemoryRequirements.alignment);

        if (memoryRequirements.memoryTypeBits != 0)
            assert(memoryRequirements.memoryTypeBits == subresourceMemoryRequirements.memoryTypeBits);
        memoryRequirements.memoryTypeBits = subresourceMemoryRequirements.memoryTypeBits;
    }

    vko::DeviceMemory memory{ m_device, m_physicalDevice, memoryRequirements, memoryPropertiesFlags };

    for (size_t i = 0; i < subresourceCount; i++)
        buffers[i].bindMemory(memory, i * alignedSize);

    Buffer bufferResource{
        .memory = nstl::move(memory),
        .buffers = nstl::move(buffers),
        .metadata = nstl::move(metadata),
        .size = size,
        .alignedSize = alignedSize,
    };

    return { m_buffers.add(nstl::move(bufferResource)) };
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, nstl::blob_view bytes, size_t offset)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(handle);

    Buffer const* buffer = getBuffer(handle);
    assert(buffer);

    return uploadBuffer(*buffer, bytes.data(), bytes.size(), offset);
}

size_t vkgfx::ResourceManager::getBufferSize(BufferHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(handle);
    auto buffer = getBuffer(handle);
    assert(buffer);
    return buffer->size;
}

vkgfx::Buffer* vkgfx::ResourceManager::getBuffer(BufferHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_buffers.get(handle);
}

void vkgfx::ResourceManager::removeBuffer(BufferHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_buffers.remove(handle);
}

vkgfx::Buffer const* vkgfx::ResourceManager::getBuffer(BufferHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_buffers.get(handle);
}

vkgfx::ShaderModuleHandle vkgfx::ResourceManager::createShaderModule(nstl::blob_view bytes, vko::ShaderModuleType type, nstl::string entryPoint)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return { m_shaderModules.add(vko::ShaderModule{ m_device, bytes, type, nstl::move(entryPoint) }) };
}

void vkgfx::ResourceManager::removeShaderModule(ShaderModuleHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_shaderModules.remove(handle);
}

vkgfx::SamplerHandle vkgfx::ResourceManager::createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return { m_samplers.add(vko::Sampler{ m_device, magFilter, minFilter, wrapU, wrapV }) };
}

vko::Sampler* vkgfx::ResourceManager::getSampler(SamplerHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_samplers.get(handle);
}

void vkgfx::ResourceManager::removeSampler(SamplerHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_samplers.remove(handle);
}

vko::Sampler const* vkgfx::ResourceManager::getSampler(SamplerHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_samplers.get(handle);
}

vkgfx::TextureHandle vkgfx::ResourceManager::createTexture(Texture texture)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return { m_textures.add(nstl::move(texture)) };
}

void vkgfx::ResourceManager::updateTexture(TextureHandle handle, Texture texture)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(handle);
    auto item = getTexture(handle);
    assert(item);
    *item = nstl::move(texture);
}

vkgfx::Texture* vkgfx::ResourceManager::getTexture(TextureHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_textures.get(handle);
}

void vkgfx::ResourceManager::removeTexture(TextureHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_textures.remove(handle);
}

vkgfx::Texture const* vkgfx::ResourceManager::getTexture(TextureHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_textures.get(handle);
}

vkgfx::MaterialHandle vkgfx::ResourceManager::createMaterial(Material material)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return { m_materials.add(nstl::move(material)) };
}

void vkgfx::ResourceManager::updateMaterial(MaterialHandle handle, Material material)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(handle);
    auto item = getMaterial(handle);
    assert(item);
    *item = nstl::move(material);
}

vkgfx::Material* vkgfx::ResourceManager::getMaterial(MaterialHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_materials.get(handle);
}

void vkgfx::ResourceManager::removeMaterial(MaterialHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_materials.remove(handle);
}

vkgfx::Material const* vkgfx::ResourceManager::getMaterial(MaterialHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_materials.get(handle);
}

void vkgfx::ResourceManager::reserveMoreMeshes(size_t size)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_meshes.reserve(m_meshes.size() + size);
}

vkgfx::MeshHandle vkgfx::ResourceManager::createMesh(Mesh mesh)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return { m_meshes.add(nstl::move(mesh)) };
}

void vkgfx::ResourceManager::updateMesh(MeshHandle handle, Mesh mesh)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(handle);
    auto item = getMesh(handle);
    assert(item);
    *item = nstl::move(mesh);
}

vkgfx::Mesh* vkgfx::ResourceManager::getMesh(MeshHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_meshes.get(handle);
}

void vkgfx::ResourceManager::removeMesh(MeshHandle handle)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    m_meshes.remove(handle);
}

vkgfx::Mesh const* vkgfx::ResourceManager::getMesh(MeshHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_meshes.get(handle);
}

vkgfx::DescriptorSetLayoutHandle vkgfx::ResourceManager::getOrCreateDescriptorSetLayout(DescriptorSetLayoutKey const& key)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    if (auto it = m_descriptorSetLayoutHandles.find(key); it != m_descriptorSetLayoutHandles.end())
        return it->value();

    return createDescriptorSetLayout(key);
}

vko::DescriptorSetLayout const& vkgfx::ResourceManager::getDescriptorSetLayout(DescriptorSetLayoutHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_descriptorSetLayouts[handle.index];
}

vkgfx::PipelineLayoutHandle vkgfx::ResourceManager::getOrCreatePipelineLayout(PipelineLayoutKey const& key)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    if (auto it = m_pipelineLayoutHandles.find(key); it != m_pipelineLayoutHandles.end())
        return it->value();

    return createPipelineLayout(key);
}

vko::PipelineLayout const& vkgfx::ResourceManager::getPipelineLayout(PipelineLayoutHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_pipelineLayouts[handle.index];
}

vkgfx::PipelineHandle vkgfx::ResourceManager::getOrCreatePipeline(PipelineKey const& key)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    if (auto it = m_pipelineHandles.find(key); it != m_pipelineHandles.end())
        return it->value();

    return createPipeline(key);
}

vko::Pipeline const& vkgfx::ResourceManager::getPipeline(PipelineHandle handle) const
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return m_pipelines[handle.index];
}

void vkgfx::ResourceManager::uploadBuffer(Buffer const& buffer, void const* data, size_t dataSize, size_t offset)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(buffer.size - offset >= dataSize);

    if (buffer.metadata.location == BufferLocation::DeviceLocal)
    {
        BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        stagingBuffer.memory().copyFrom(data, dataSize);

        OneTimeCommandBuffer commandBuffer{ *m_uploadCommandPool, m_uploadQueue };
        assert(!buffer.metadata.isMutable); // Not implemented yet
        vko::Buffer::copy(commandBuffer.getHandle(), stagingBuffer.buffer(), 0, buffer.buffers[0], offset, dataSize);
        commandBuffer.submit();
    }
    else
    {
        size_t index = buffer.metadata.isMutable ? m_currentSubresourceIndex : 0;
        size_t subresourceOffset = index * buffer.alignedSize;
        assert(subresourceOffset + buffer.alignedSize <= buffer.memory.getRequirements().size);
        buffer.memory.copyFrom(data, dataSize, subresourceOffset + offset);
    }
}

void vkgfx::ResourceManager::uploadBuffer(Buffer const& buffer, nstl::blob_view bytes, size_t offset)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    return uploadBuffer(buffer, bytes.data(), bytes.size(), offset);
}

void vkgfx::ResourceManager::uploadImage(Image const& image, void const* data, size_t dataSize)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    assert(image.metadata.byteSize == dataSize);

    auto width = static_cast<uint32_t>(image.metadata.width);
    auto height = static_cast<uint32_t>(image.metadata.height);

    BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    stagingBuffer.memory().copyFrom(data, dataSize);

    OneTimeCommandBuffer commandBuffer{ *m_uploadCommandPool, m_uploadQueue };
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // TODO extract to some class
    copyBufferToImage(commandBuffer.getHandle(), stagingBuffer.buffer().getHandle(), image.image.getHandle(), width, height);
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    commandBuffer.submit();
}

vkgfx::DescriptorSetLayoutHandle vkgfx::ResourceManager::createDescriptorSetLayout(DescriptorSetLayoutKey const& key)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    DescriptorSetLayoutHandle handle;
    handle.index = m_descriptorSetLayoutHandles.size();

    // TODO avoid filling this structure
    vko::DescriptorSetConfiguration config;
    config.hasBuffer = key.uniformConfig.hasBuffer;
    config.hasTexture = key.uniformConfig.hasAlbedoTexture;
    config.hasNormalMap = key.uniformConfig.hasNormalMap;
    config.hasShadowMap = key.uniformConfig.hasShadowMap;

    m_descriptorSetLayouts.emplace_back(m_device, nstl::move(config));
    m_descriptorSetLayoutHandles[key] = handle;

    return handle;
}

vkgfx::PipelineLayoutHandle vkgfx::ResourceManager::createPipelineLayout(PipelineLayoutKey const& key)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    PipelineLayoutHandle handle;
    handle.index = m_pipelineLayouts.size();

    nstl::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    for (UniformConfiguration const& uniformConfig : key.uniformConfigs)
    {
        DescriptorSetLayoutKey descriptorSetLayoutKey;
        descriptorSetLayoutKey.uniformConfig = uniformConfig;
        DescriptorSetLayoutHandle descriptorSetLayoutHandle = getOrCreateDescriptorSetLayout(descriptorSetLayoutKey);
        descriptorSetLayouts.push_back(m_descriptorSetLayouts[descriptorSetLayoutHandle.index].getHandle());
    }

    nstl::vector<VkPushConstantRange> pushConstantRanges;
    for (PushConstantRange const& range : key.pushConstantRanges)
    {
        VkPushConstantRange& vkRange = pushConstantRanges.emplace_back();
        vkRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO configure
        vkRange.offset = range.offset;
        vkRange.size = range.size;
    }

    m_pipelineLayouts.emplace_back(m_device, nstl::move(descriptorSetLayouts), nstl::move(pushConstantRanges));
    m_pipelineLayoutHandles[key] = handle;

    return handle;
}

vkgfx::PipelineHandle vkgfx::ResourceManager::createPipeline(PipelineKey const& key)
{
    MEMORY_TRACKING_SCOPE(scopeId);

    PipelineHandle handle;
    handle.index = m_pipelines.size();

    // TODO make use of key.vertexConfig.topology
    assert(key.vertexConfig.topology == VertexTopology::Triangles);

    vko::Pipeline::Config config;
    // TODO merge these flags with Pipeline::Config
    config.cullBackFaces = key.renderConfig.cullBackfaces;
    config.wireframe = key.renderConfig.wireframe;
    config.depthTest = key.renderConfig.depthTest;
    config.alphaBlending = key.renderConfig.alphaBlending;
    config.depthBias = key.renderConfig.depthBias;

    for (size_t i = 0; i < key.vertexConfig.bindings.size(); i++)
    {
        VertexConfiguration::Binding const& binding = key.vertexConfig.bindings[i];
        VkVertexInputBindingDescription& desc = config.bindingDescriptions.emplace_back();
        desc.binding = i;
        desc.stride = binding.stride;
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // TODO configure
    }

    for (VertexConfiguration::Attribute const& attribute : key.vertexConfig.attributes)
    {
        VkVertexInputAttributeDescription& desc = config.attributeDescriptions.emplace_back();
        desc.binding = attribute.binding;
        desc.location = attribute.location;
        desc.format = ::vulkanizeAttributeFormat(attribute.type);
        desc.offset = attribute.offset;
    }

    PipelineLayoutKey layoutKey;
    layoutKey.uniformConfigs = key.uniformConfigs;
    layoutKey.pushConstantRanges = key.pushConstantRanges;

    PipelineLayoutHandle pipelineLayoutHandle = getOrCreatePipelineLayout(layoutKey);

    nstl::vector<vko::ShaderModule const*> shaderModules;
    for (ShaderModuleHandle const& handle : key.shaderHandles)
    {
        assert(handle);
        vko::ShaderModule const* shaderModule = m_shaderModules.get(handle);
        assert(shaderModule);
        shaderModules.push_back(shaderModule);
    }

    vko::PipelineLayout const& pipelineLayout = m_pipelineLayouts[pipelineLayoutHandle.index];

    m_pipelines.emplace_back(m_device, pipelineLayout, key.isShadowmap ? m_shadowmapRenderPass : m_renderPass, shaderModules, config);
    m_pipelineHandles[key] = handle;

    return handle;
}
