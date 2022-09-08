#include "ResourceManager.h"
#include "BufferWithMemory.h"
#include "wrapper/CommandBuffers.h"
#include "wrapper/CommandPool.h"
#include "wrapper/Queue.h"
#include "wrapper/ShaderModule.h"
#include "wrapper/DescriptorSetLayout.h"
#include "wrapper/PipelineLayout.h"
#include "wrapper/Pipeline.h"
#include "Image.h"
#include "Buffer.h"
#include "Sampler.h"
#include "Handles.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "PipelineKey.h"

namespace
{
    class OneTimeCommandBuffer
    {
    public:
        OneTimeCommandBuffer(vko::CommandPool const& commandPool, vko::Queue const& queue)
            : m_queue(queue)
            , m_buffers(commandPool.createCommandBuffers(1))
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

    VkFormat getVulkanAttributeFormat(vkgfx::AttributeType type)
    {
        switch (type)
        {
        case vkgfx::AttributeType::Vec2f:
            return VK_FORMAT_R32G32_SFLOAT;
        case vkgfx::AttributeType::Vec3f:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case vkgfx::AttributeType::Vec4f:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        }

        throw std::invalid_argument("type");
    }
}

// TODO move to another file?
namespace vkgfx
{
    struct DescriptorSetLayoutKey
    {
        UniformConfiguration uniformConfig;

        auto operator<=>(DescriptorSetLayoutKey const&) const = default;
    };

    struct PipelineLayoutKey
    {
        std::vector<UniformConfiguration> uniformConfigs;
        std::vector<PushConstantRange> pushConstantRanges;

        auto operator<=>(PipelineLayoutKey const&) const = default;
    };
}

vkgfx::ResourceManager::ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue, vko::RenderPass const& renderPass, std::size_t width, std::size_t height)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_uploadCommandPool(uploadCommandPool)
    , m_uploadQueue(uploadQueue)
    , m_renderPass(renderPass)
    , m_width(width)
    , m_height(height)
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

        throw std::invalid_argument("format");
    }(metadata.format);

    auto width = static_cast<uint32_t>(metadata.width);
    auto height = static_cast<uint32_t>(metadata.height);

    vko::Image vkImage{ m_device, width, height, vkFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT };
    vko::DeviceMemory vkImageMemory{ m_device, m_physicalDevice, vkImage.getMemoryRequirements(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
    vkImage.bindMemory(vkImageMemory);

    vko::ImageView vkImageView = vkImage.createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

    ImageHandle handle;
    handle.index = m_images.size();

    m_images.emplace_back(std::move(vkImageMemory), std::move(vkImage), std::move(vkImageView), std::move(metadata));

    return handle;
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, void const* data, std::size_t dataSize)
{
    Image const& image = m_images[handle.index];

    auto width = static_cast<uint32_t>(image.metadata.width);
    auto height = static_cast<uint32_t>(image.metadata.height);

    vkr::BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    stagingBuffer.memory().copyFrom(data, dataSize);

    OneTimeCommandBuffer commandBuffer{ m_uploadCommandPool, m_uploadQueue };
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // TODO extract to some class
    copyBufferToImage(commandBuffer.getHandle(), stagingBuffer.buffer().getHandle(), image.image.getHandle(), width, height);
    transitionImageLayout(commandBuffer.getHandle(), image.image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    commandBuffer.submit();
}

void vkgfx::ResourceManager::uploadImage(ImageHandle handle, std::span<unsigned char const> bytes)
{
    return uploadImage(handle, bytes.data(), bytes.size());
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

    vko::Buffer buffer{m_device, size, bufferUsageFlags };
    vko::DeviceMemory memory{ m_device, m_physicalDevice, buffer.getMemoryRequirements(), memoryPropertiesFlags };
    buffer.bindMemory(memory);

    BufferHandle handle;
    handle.index = m_buffers.size();

    m_buffers.emplace_back(std::move(memory), std::move(buffer), std::move(metadata));

    return handle;
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, void const* data, std::size_t dataSize, std::size_t offset)
{
    Buffer const& buffer = m_buffers[handle.index];

    if (buffer.metadata.location == BufferLocation::DeviceLocal)
    {
        vkr::BufferWithMemory stagingBuffer{ m_device, m_physicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
        stagingBuffer.memory().copyFrom(data, dataSize, offset);

        OneTimeCommandBuffer commandBuffer{ m_uploadCommandPool, m_uploadQueue };
        vko::Buffer::copy(commandBuffer.getHandle(), stagingBuffer.buffer(), buffer.buffer);
        commandBuffer.submit();
    }
    else
    {
        buffer.memory.copyFrom(data, dataSize, offset);
    }
}

void vkgfx::ResourceManager::uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes, std::size_t offset)
{
    return uploadBuffer(handle, bytes.data(), bytes.size(), offset);
}

vkgfx::ShaderModuleHandle vkgfx::ResourceManager::createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint)
{
    ShaderModuleHandle handle;
    handle.index = m_shaderModules.size();

    m_shaderModules.emplace_back(m_device, bytes, type, std::move(entryPoint));

    return handle;
}

vkgfx::SamplerHandle vkgfx::ResourceManager::createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV)
{
    vko::Sampler sampler{ m_device, magFilter, minFilter, wrapU, wrapV };

    SamplerHandle handle;
    handle.index = m_samplers.size();

    m_samplers.emplace_back(std::move(sampler));

    return handle;
}

vkgfx::TextureHandle vkgfx::ResourceManager::createTexture(Texture texture)
{
    TextureHandle handle;
    handle.index = m_textures.size();

    m_textures.push_back(std::move(texture));

    return handle;
}

vkgfx::MaterialHandle vkgfx::ResourceManager::createMaterial(Material material)
{
    MaterialHandle handle;
    handle.index = m_materials.size();

    m_materials.push_back(std::move(material));

    return handle;
}

vkgfx::MeshHandle vkgfx::ResourceManager::createMesh(Mesh mesh)
{
    MeshHandle handle;
    handle.index = m_meshes.size();

    m_meshes.push_back(std::move(mesh));

    return handle;
}

vkgfx::PipelineHandle vkgfx::ResourceManager::getOrCreatePipeline(PipelineKey const& key)
{
    if (auto it = m_pipelineHandles.find(key); it != m_pipelineHandles.end())
        return it->second;

    return createPipeline(key);
}

vkgfx::DescriptorSetLayoutHandle vkgfx::ResourceManager::getOrCreateDescriptorSetLayout(DescriptorSetLayoutKey const& key)
{
    if (auto it = m_descriptorSetLayoutHandles.find(key); it != m_descriptorSetLayoutHandles.end())
        return it->second;

    return createDescriptorSetLayout(key);
}

vkgfx::DescriptorSetLayoutHandle vkgfx::ResourceManager::createDescriptorSetLayout(DescriptorSetLayoutKey const& key)
{
    DescriptorSetLayoutHandle handle;
    handle.index = m_descriptorSetLayoutHandles.size();

    // TODO avoid filling this structure
    vko::DescriptorSetConfiguration config;
    config.hasTexture = key.uniformConfig.hasAlbedoTexture;
    config.hasNormalMap = key.uniformConfig.hasNormalMap;

    m_descriptorSetLayouts.emplace_back(m_device, std::move(config));
    m_descriptorSetLayoutHandles[key] = handle;

    return handle;
}

vkgfx::PipelineLayoutHandle vkgfx::ResourceManager::getOrCreatePipelineLayout(PipelineLayoutKey const& key)
{
    if (auto it = m_pipelineLayoutHandles.find(key); it != m_pipelineLayoutHandles.end())
        return it->second;

    return createPipelineLayout(key);
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
    config.extent = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; // TODO fix
    config.cullBackFaces = key.renderConfig.cullBackfaces;
    config.wireframe = key.renderConfig.wireframe;

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
        desc.format = ::getVulkanAttributeFormat(attribute.type);
        desc.offset = attribute.offset;
    }

    PipelineLayoutKey layoutKey;
    layoutKey.uniformConfigs = key.uniformConfigs;
    layoutKey.pushConstantRanges = key.pushConstantRanges;

    PipelineLayoutHandle pipelineLayoutHandle = getOrCreatePipelineLayout(layoutKey);

    std::vector<vko::ShaderModule const*> shaderModules;
    for (ShaderModuleHandle const& handle : key.shaderHandles)
        shaderModules.push_back(&m_shaderModules[handle.index]);

    vko::PipelineLayout const& pipelineLayout = m_pipelineLayouts[pipelineLayoutHandle.index];

    m_pipelines.emplace_back(m_device, pipelineLayout, m_renderPass, shaderModules, config);
    m_pipelineHandles[key] = handle;

    return handle;
}
