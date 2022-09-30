#include "ResourceManager.h"

#include "wrapper/CommandBuffers.h"
#include "wrapper/CommandPool.h"
#include "wrapper/Queue.h"
#include "wrapper/ShaderModule.h"
#include "wrapper/DescriptorSetLayout.h"
#include "wrapper/PipelineLayout.h"
#include "wrapper/Pipeline.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Sampler.h"

#include "Image.h"
#include "Buffer.h"
#include "Handles.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "PipelineKey.h"

#include <cassert>

namespace
{
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
            m_buffer.reset();
            m_memory.reset();
        }

        BufferWithMemory(BufferWithMemory&& rhs) = default;
        BufferWithMemory& operator=(BufferWithMemory&& rhs) = default;

        vko::Buffer const& buffer() const { return *m_buffer; }
        vko::DeviceMemory const& memory() const { return *m_memory; }

    private:
        // TODO remove this nasty hack
        std::optional<vko::Buffer> m_buffer;
        std::optional<vko::DeviceMemory> m_memory;
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
            throw std::invalid_argument("unsupported layout transition!");
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
        }

        throw std::invalid_argument("type");
    }

    std::size_t alignSize(std::size_t originalSize, std::size_t alignment)
    {
        if (alignment == 0)
            return originalSize;

        return (originalSize + alignment - 1) & ~(alignment - 1);
    }
}

vkgfx::ResourceManager::ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue, vko::RenderPass const& renderPass, std::size_t resourceCount)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_uploadCommandPool(uploadCommandPool)
    , m_uploadQueue(uploadQueue)
    , m_renderPass(renderPass)
    , m_resourceCount(resourceCount)
{

}

vkgfx::ResourceManager::~ResourceManager() = default;

vkgfx::ImageHandle vkgfx::ResourceManager::createImage(ImageMetadata metadata)
{
    // TODO use SRGB for textures data and UNORM for normal maps
    VkFormat vkFormat = [](ImageFormat format) {
        switch (format)
        {
        case ImageFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        }

        assert(false);
        throw std::invalid_argument("format");
    }(metadata.format);

    std::size_t bytesPerPixel = [](ImageFormat format)
    {
        switch (format)
        {
        case ImageFormat::R8G8B8A8:
            return 4;
        case ImageFormat::R8G8B8:
            return 3;
        }

        assert(false);
        throw std::invalid_argument("format");
    }(metadata.format);

    auto width = static_cast<uint32_t>(metadata.width);
    auto height = static_cast<uint32_t>(metadata.height);

    vko::Image vkImage{ m_device, width, height, vkFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
    vko::DeviceMemory vkImageMemory{ m_device, m_physicalDevice, vkImage.getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
    vkImage.bindMemory(vkImageMemory);

    vko::ImageView vkImageView{ m_device, vkImage, VK_IMAGE_ASPECT_COLOR_BIT };

    Image imageResource = {
        .memory = std::move(vkImageMemory),
        .image = std::move(vkImage),
        .imageView = std::move(vkImageView),
        .metadata = std::move(metadata),
        .byteSize = metadata.width * metadata.height * bytesPerPixel,
    };

    return { m_images.add(std::move(imageResource)) };
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, void const* data, std::size_t dataSize)
{
    assert(handle);
    Image const* image = getImage(handle);
    assert(image);

    return uploadImage(*image, data, dataSize);
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, std::span<unsigned char const> bytes)
{
    return uploadImage(handle, bytes.data(), bytes.size());
}

vkgfx::Image* vkgfx::ResourceManager::getImage(ImageHandle handle)
{
    return m_images.get(handle);
}

void vkgfx::ResourceManager::removeImage(ImageHandle handle)
{
    m_images.remove(handle);
}

vkgfx::Image const* vkgfx::ResourceManager::getImage(ImageHandle handle) const
{
    return m_images.get(handle);
}

vkgfx::BufferHandle vkgfx::ResourceManager::createBuffer(std::size_t size, BufferMetadata metadata)
{
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

    std::size_t alignedSize = alignSize(size, m_physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment);

    std::size_t totalBufferSize = size;
    if (metadata.isMutable)
        totalBufferSize = alignedSize * m_resourceCount;

    vko::Buffer buffer{m_device, totalBufferSize, bufferUsageFlags };
    vko::DeviceMemory memory{ m_device, m_physicalDevice, buffer.getMemoryRequirements(), memoryPropertiesFlags };
    buffer.bindMemory(memory);

    Buffer bufferResource{
        .memory = std::move(memory),
        .buffer = std::move(buffer),
        .metadata = std::move(metadata),
        .size = size,
        .realSize = totalBufferSize,
    };

    if (metadata.isMutable)
    {
        bufferResource.stagingBuffer.resize(size);
        bufferResource.alignedSize = alignedSize;
    }

    return { m_buffers.add(std::move(bufferResource)) };
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, void const* data, std::size_t dataSize)
{
    assert(handle);

    Buffer const* buffer = getBuffer(handle);
    assert(buffer);

    assert(!buffer->metadata.isMutable);

    return uploadBuffer(*buffer, data, dataSize, 0);
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, std::span<std::byte const> bytes)
{
    return uploadBuffer(handle, bytes.data(), bytes.size());
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes)
{
    return uploadBuffer(handle, bytes.data(), bytes.size());
}

void vkgfx::ResourceManager::uploadDynamicBufferToStaging(BufferHandle handle, void const* data, std::size_t dataSize, std::size_t offset)
{
    assert(handle);

    Buffer* buffer = getBuffer(handle);
    assert(buffer);

    assert(buffer->metadata.isMutable);
    assert(buffer->stagingBuffer.size() - offset >= dataSize);

    memcpy(buffer->stagingBuffer.data() + offset, data, dataSize);

    std::size_t start = offset;
    std::size_t end = offset + dataSize;

    buffer->stagingDirtyStart = std::min(buffer->stagingDirtyStart, start);
    buffer->stagingDirtyEnd = std::max(buffer->stagingDirtyEnd, end);
}

void vkgfx::ResourceManager::transferDynamicBuffersFromStaging(std::size_t resourceIndex)
{
    assert(resourceIndex < m_resourceCount);

    for (Buffer& buffer : m_buffers)
    {
        if (!buffer.metadata.isMutable)
            continue;

        std::size_t dirtyOffset = buffer.stagingDirtyStart;
        std::size_t dirtySize = buffer.stagingDirtyEnd - buffer.stagingDirtyStart;

        if (dirtySize == 0)
            continue;

        std::size_t bufferOffset = buffer.alignedSize * resourceIndex;

        void* data = buffer.stagingBuffer.data() + dirtyOffset;
        std::size_t size = dirtySize;

        uploadBuffer(buffer, data, size, bufferOffset + dirtyOffset);

        buffer.stagingDirtyStart = 0;
        buffer.stagingDirtyEnd = 0;
    }
}

std::size_t vkgfx::ResourceManager::getBufferSize(BufferHandle handle) const
{
    assert(handle);
    auto buffer = getBuffer(handle);
    assert(buffer);
    return buffer->size;
}

vkgfx::Buffer* vkgfx::ResourceManager::getBuffer(BufferHandle handle)
{
    return m_buffers.get(handle);
}

void vkgfx::ResourceManager::removeBuffer(BufferHandle handle)
{
    m_buffers.remove(handle);
}

vkgfx::Buffer const* vkgfx::ResourceManager::getBuffer(BufferHandle handle) const
{
    return m_buffers.get(handle);
}

vkgfx::ShaderModuleHandle vkgfx::ResourceManager::createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint)
{
    return { m_shaderModules.add(vko::ShaderModule{ m_device, bytes, type, std::move(entryPoint) }) };
}

void vkgfx::ResourceManager::removeShaderModule(ShaderModuleHandle handle)
{
    m_shaderModules.remove(handle);
}

vkgfx::SamplerHandle vkgfx::ResourceManager::createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV)
{
    return { m_samplers.add(vko::Sampler{ m_device, magFilter, minFilter, wrapU, wrapV }) };
}

vko::Sampler* vkgfx::ResourceManager::getSampler(SamplerHandle handle)
{
    return m_samplers.get(handle);
}

void vkgfx::ResourceManager::removeSampler(SamplerHandle handle)
{
    m_samplers.remove(handle);
}

vko::Sampler const* vkgfx::ResourceManager::getSampler(SamplerHandle handle) const
{
    return m_samplers.get(handle);
}

vkgfx::TextureHandle vkgfx::ResourceManager::createTexture(Texture texture)
{
    return { m_textures.add(std::move(texture)) };
}

void vkgfx::ResourceManager::updateTexture(TextureHandle handle, Texture texture)
{
    assert(handle);
    auto item = getTexture(handle);
    assert(item);
    *item = std::move(texture);
}

vkgfx::Texture* vkgfx::ResourceManager::getTexture(TextureHandle handle)
{
    return m_textures.get(handle);
}

void vkgfx::ResourceManager::removeTexture(TextureHandle handle)
{
    m_textures.remove(handle);
}

vkgfx::Texture const* vkgfx::ResourceManager::getTexture(TextureHandle handle) const
{
    return m_textures.get(handle);
}

vkgfx::MaterialHandle vkgfx::ResourceManager::createMaterial(Material material)
{
    return { m_materials.add(std::move(material)) };
}

void vkgfx::ResourceManager::updateMaterial(MaterialHandle handle, Material material)
{
    assert(handle);
    auto item = getMaterial(handle);
    assert(item);
    *item = std::move(material);
}

vkgfx::Material* vkgfx::ResourceManager::getMaterial(MaterialHandle handle)
{
    return m_materials.get(handle);
}

void vkgfx::ResourceManager::removeMaterial(MaterialHandle handle)
{
    m_materials.remove(handle);
}

vkgfx::Material const* vkgfx::ResourceManager::getMaterial(MaterialHandle handle) const
{
    return m_materials.get(handle);
}

vkgfx::MeshHandle vkgfx::ResourceManager::createMesh(Mesh mesh)
{
    return { m_meshes.add(std::move(mesh)) };
}

void vkgfx::ResourceManager::updateMesh(MeshHandle handle, Mesh mesh)
{
    assert(handle);
    auto item = getMesh(handle);
    assert(item);
    *item = std::move(mesh);
}

vkgfx::Mesh* vkgfx::ResourceManager::getMesh(MeshHandle handle)
{
    return m_meshes.get(handle);
}

void vkgfx::ResourceManager::removeMesh(MeshHandle handle)
{
    m_meshes.remove(handle);
}

vkgfx::Mesh const* vkgfx::ResourceManager::getMesh(MeshHandle handle) const
{
    return m_meshes.get(handle);
}

vkgfx::DescriptorSetLayoutHandle vkgfx::ResourceManager::getOrCreateDescriptorSetLayout(DescriptorSetLayoutKey const& key)
{
    if (auto it = m_descriptorSetLayoutHandles.find(key); it != m_descriptorSetLayoutHandles.end())
        return it->second;

    return createDescriptorSetLayout(key);
}

vko::DescriptorSetLayout const& vkgfx::ResourceManager::getDescriptorSetLayout(DescriptorSetLayoutHandle handle) const
{
    return m_descriptorSetLayouts[handle.index];
}

vkgfx::PipelineLayoutHandle vkgfx::ResourceManager::getOrCreatePipelineLayout(PipelineLayoutKey const& key)
{
    if (auto it = m_pipelineLayoutHandles.find(key); it != m_pipelineLayoutHandles.end())
        return it->second;

    return createPipelineLayout(key);
}

vko::PipelineLayout const& vkgfx::ResourceManager::getPipelineLayout(PipelineLayoutHandle handle) const
{
    return m_pipelineLayouts[handle.index];
}

vkgfx::PipelineHandle vkgfx::ResourceManager::getOrCreatePipeline(PipelineKey const& key)
{
    if (auto it = m_pipelineHandles.find(key); it != m_pipelineHandles.end())
        return it->second;

    return createPipeline(key);
}

vko::Pipeline const& vkgfx::ResourceManager::getPipeline(PipelineHandle handle) const
{
    return m_pipelines[handle.index];
}

void vkgfx::ResourceManager::uploadBuffer(Buffer const& buffer, void const* data, std::size_t dataSize, std::size_t offset)
{
    assert(buffer.realSize - offset >= dataSize);

    if (buffer.metadata.location == BufferLocation::DeviceLocal)
    {
        BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        stagingBuffer.memory().copyFrom(data, dataSize);

        OneTimeCommandBuffer commandBuffer{ m_uploadCommandPool, m_uploadQueue };
        vko::Buffer::copy(commandBuffer.getHandle(), stagingBuffer.buffer(), 0, buffer.buffer, offset, dataSize);
        commandBuffer.submit();
    }
    else
    {
        buffer.memory.copyFrom(data, dataSize, offset);
    }
}

void vkgfx::ResourceManager::uploadBuffer(Buffer const& buffer, std::span<unsigned char const> bytes, std::size_t offset)
{
    return uploadBuffer(buffer, bytes.data(), bytes.size(), offset);
}

void vkgfx::ResourceManager::uploadImage(Image const& image, void const* data, std::size_t dataSize)
{
    assert(image.byteSize == dataSize);

    auto width = static_cast<uint32_t>(image.metadata.width);
    auto height = static_cast<uint32_t>(image.metadata.height);

    BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    stagingBuffer.memory().copyFrom(data, dataSize);

    OneTimeCommandBuffer commandBuffer{ m_uploadCommandPool, m_uploadQueue };
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // TODO extract to some class
    copyBufferToImage(commandBuffer.getHandle(), stagingBuffer.buffer().getHandle(), image.image.getHandle(), width, height);
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    commandBuffer.submit();
}

vkgfx::DescriptorSetLayoutHandle vkgfx::ResourceManager::createDescriptorSetLayout(DescriptorSetLayoutKey const& key)
{
    DescriptorSetLayoutHandle handle;
    handle.index = m_descriptorSetLayoutHandles.size();

    // TODO avoid filling this structure
    vko::DescriptorSetConfiguration config;
    config.hasBuffer = key.uniformConfig.hasBuffer;
    config.hasTexture = key.uniformConfig.hasAlbedoTexture;
    config.hasNormalMap = key.uniformConfig.hasNormalMap;

    m_descriptorSetLayouts.emplace_back(m_device, std::move(config));
    m_descriptorSetLayoutHandles[key] = handle;

    return handle;
}

vkgfx::PipelineLayoutHandle vkgfx::ResourceManager::createPipelineLayout(PipelineLayoutKey const& key)
{
    PipelineLayoutHandle handle;
    handle.index = m_pipelineLayouts.size();

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    for (UniformConfiguration const& uniformConfig : key.uniformConfigs)
    {
        DescriptorSetLayoutKey descriptorSetLayoutKey;
        descriptorSetLayoutKey.uniformConfig = uniformConfig;
        DescriptorSetLayoutHandle descriptorSetLayoutHandle = getOrCreateDescriptorSetLayout(descriptorSetLayoutKey);
        descriptorSetLayouts.push_back(m_descriptorSetLayouts[descriptorSetLayoutHandle.index].getHandle());
    }

    std::vector<VkPushConstantRange> pushConstantRanges;
    for (PushConstantRange const& range : key.pushConstantRanges)
    {
        VkPushConstantRange& vkRange = pushConstantRanges.emplace_back();
        vkRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO configure
        vkRange.offset = range.offset;
        vkRange.size = range.size;
    }

    m_pipelineLayouts.emplace_back(m_device, std::move(descriptorSetLayouts), std::move(pushConstantRanges));
    m_pipelineLayoutHandles[key] = handle;

    return handle;
}

vkgfx::PipelineHandle vkgfx::ResourceManager::createPipeline(PipelineKey const& key)
{
    PipelineHandle handle;
    handle.index = m_pipelines.size();

    vko::Pipeline::Config config;
    // TODO merge these flags with Pipeline::Config
    config.cullBackFaces = key.renderConfig.cullBackfaces;
    config.wireframe = key.renderConfig.wireframe;
    config.depthTest = key.renderConfig.depthTest;
    config.alphaBlending = key.renderConfig.alphaBlending;

    for (std::size_t i = 0; i < key.vertexConfig.bindings.size(); i++)
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

    std::vector<vko::ShaderModule const*> shaderModules;
    for (ShaderModuleHandle const& handle : key.shaderHandles)
    {
        assert(handle);
        vko::ShaderModule const* shaderModule = m_shaderModules.get(handle);
        assert(shaderModule);
        shaderModules.push_back(shaderModule);
    }

    vko::PipelineLayout const& pipelineLayout = m_pipelineLayouts[pipelineLayoutHandle.index];

    m_pipelines.emplace_back(m_device, pipelineLayout, m_renderPass, shaderModules, config);
    m_pipelineHandles[key] = handle;

    return handle;
}
