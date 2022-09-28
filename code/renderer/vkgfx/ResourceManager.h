#pragma once

#include <span>
#include <vector>
#include <string>
#include <map>

#include "ResourceContainer.h"

namespace vko
{
    class Device;
    class PhysicalDevice;
    class CommandPool;
    class Queue;
    class RenderPass;

    class ShaderModule;
    class DescriptorSetLayout;
    class PipelineLayout;
    class Pipeline;

    enum class ShaderModuleType;
    enum class SamplerFilterMode;
    enum class SamplerWrapMode;
}

namespace vkgfx
{
    struct Image;
    struct ImageHandle;
    struct ImageMetadata;

    struct Buffer;
    struct BufferHandle;
    struct BufferMetadata;

    struct ShaderModuleHandle;

    struct Sampler;
    struct SamplerHandle;

    struct Texture;
    struct TextureHandle;

    struct Material;
    struct MaterialHandle;

    struct Mesh;
    struct MeshHandle;

    struct DescriptorSetLayoutHandle;
    struct DescriptorSetLayoutKey;

    struct PipelineLayoutHandle;
    struct PipelineLayoutKey;

    struct PipelineHandle;
    struct PipelineKey;

    class ResourceManager
    {
    public:
        ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue, vko::RenderPass const& renderPass, std::size_t resourceCount);
        ~ResourceManager();

        ImageHandle createImage(ImageMetadata metadata);
        void uploadImage(ImageHandle handle, void const* data, std::size_t dataSize);
        void uploadImage(ImageHandle handle, std::span<unsigned char const> bytes);
        Image const& getImage(ImageHandle handle) const;

        BufferHandle createBuffer(std::size_t size, BufferMetadata metadata);
        void uploadBuffer(BufferHandle handle, void const* data, std::size_t dataSize);
        void uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes);
        void uploadBuffer(BufferHandle handle, std::span<std::byte const> bytes);
        void uploadDynamicBufferToStaging(BufferHandle handle, void const* data, std::size_t dataSize, std::size_t offset = 0);
        void transferDynamicBuffersFromStaging(std::size_t resourceIndex);
        std::size_t getBufferSize(BufferHandle handle) const;
        Buffer* getBuffer(BufferHandle handle);
        Buffer const* getBuffer(BufferHandle handle) const;

        ShaderModuleHandle createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint = "main");

        SamplerHandle createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV);
        Sampler const& getSampler(SamplerHandle handle) const;

        TextureHandle createTexture(Texture texture);
        void updateTexture(TextureHandle handle, Texture texture);
        Texture const& getTexture(TextureHandle handle) const;

        MaterialHandle createMaterial(Material material);
        void updateMaterial(MaterialHandle handle, Material material);
        Material const& getMaterial(MaterialHandle handle) const;

        MeshHandle createMesh(Mesh mesh);
        void updateMesh(MeshHandle handle, Mesh mesh);
        Mesh const& getMesh(MeshHandle handle) const;

        DescriptorSetLayoutHandle getOrCreateDescriptorSetLayout(DescriptorSetLayoutKey const& key);
        vko::DescriptorSetLayout const& getDescriptorSetLayout(DescriptorSetLayoutHandle handle) const;

        PipelineLayoutHandle getOrCreatePipelineLayout(PipelineLayoutKey const& key);
        vko::PipelineLayout const& getPipelineLayout(PipelineLayoutHandle handle) const;

        PipelineHandle getOrCreatePipeline(PipelineKey const& key);
        vko::Pipeline const& getPipeline(PipelineHandle handle) const;

    private:
        void uploadBuffer(Buffer const& buffer, void const* data, std::size_t dataSize, std::size_t offset);
        void uploadBuffer(Buffer const& buffer, std::span<unsigned char const> bytes, std::size_t offset);
        void uploadImage(Image const& image, void const* data, std::size_t dataSize);

        DescriptorSetLayoutHandle createDescriptorSetLayout(DescriptorSetLayoutKey const& key);

        PipelineLayoutHandle createPipelineLayout(PipelineLayoutKey const& key);

        PipelineHandle createPipeline(PipelineKey const& key);

    private:
        vko::Device const& m_device;
        vko::PhysicalDevice const& m_physicalDevice; // TODO replace with the allocator
        vko::CommandPool const& m_uploadCommandPool;
        vko::Queue const& m_uploadQueue;
        vko::RenderPass const& m_renderPass; // TODO remove
        std::size_t m_resourceCount = 0; // TODO rename

        std::vector<Image> m_images;
        ResourceContainer<Buffer> m_buffers;
        std::vector<vko::ShaderModule> m_shaderModules;
        std::vector<Sampler> m_samplers;

        std::vector<Texture> m_textures;
        std::vector<Material> m_materials;
        std::vector<Mesh> m_meshes;

        std::vector<vko::DescriptorSetLayout> m_descriptorSetLayouts;
        std::map<DescriptorSetLayoutKey, DescriptorSetLayoutHandle> m_descriptorSetLayoutHandles;

        std::vector<vko::PipelineLayout> m_pipelineLayouts;
        std::map<PipelineLayoutKey, PipelineLayoutHandle> m_pipelineLayoutHandles;

        std::vector<vko::Pipeline> m_pipelines;
        std::map<PipelineKey, PipelineHandle> m_pipelineHandles;
    };
}
