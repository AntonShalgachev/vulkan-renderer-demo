#pragma once

#include "vkgfx/ResourceContainer.h"

#include "nstl/vector.h"
#include "nstl/span.h"
#include "nstl/string.h"
#include "nstl/unordered_map.h"

namespace vko
{
    class Device;
    class PhysicalDevice;
    class CommandPool;
    class Queue;
    class RenderPass;

    class ShaderModule;
    class Sampler;
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
        void uploadImage(ImageHandle handle, nstl::span<unsigned char const> bytes);
        Image* getImage(ImageHandle handle);
        Image const* getImage(ImageHandle handle) const;
        void removeImage(ImageHandle handle);

        void reserveMoreBuffers(std::size_t size);
        BufferHandle createBuffer(std::size_t size, BufferMetadata metadata);
        void uploadBuffer(BufferHandle handle, void const* data, std::size_t dataSize);
        void uploadBuffer(BufferHandle handle, nstl::span<unsigned char const> bytes);
        void uploadBuffer(BufferHandle handle, nstl::span<std::byte const> bytes);
        void uploadDynamicBufferToStaging(BufferHandle handle, void const* data, std::size_t dataSize, std::size_t offset = 0);
        void transferDynamicBuffersFromStaging(std::size_t resourceIndex);
        std::size_t getBufferSize(BufferHandle handle) const;
        Buffer* getBuffer(BufferHandle handle);
        Buffer const* getBuffer(BufferHandle handle) const;
        void removeBuffer(BufferHandle handle);

        ShaderModuleHandle createShaderModule(nstl::span<unsigned char const> bytes, vko::ShaderModuleType type, nstl::string entryPoint = "main");
        void removeShaderModule(ShaderModuleHandle handle);

        SamplerHandle createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV);
        vko::Sampler* getSampler(SamplerHandle handle);
        vko::Sampler const* getSampler(SamplerHandle handle) const;
        void removeSampler(SamplerHandle handle);

        TextureHandle createTexture(Texture texture);
        void updateTexture(TextureHandle handle, Texture texture);
        Texture* getTexture(TextureHandle handle);
        Texture const* getTexture(TextureHandle handle) const;
        void removeTexture(TextureHandle handle);

        MaterialHandle createMaterial(Material material);
        void updateMaterial(MaterialHandle handle, Material material);
        Material* getMaterial(MaterialHandle handle);
        Material const* getMaterial(MaterialHandle handle) const;
        void removeMaterial(MaterialHandle handle);

        void reserveMoreMeshes(std::size_t size);
        MeshHandle createMesh(Mesh mesh);
        void updateMesh(MeshHandle handle, Mesh mesh);
        Mesh const* getMesh(MeshHandle handle) const;
        Mesh* getMesh(MeshHandle handle);
        void removeMesh(MeshHandle handle);

        DescriptorSetLayoutHandle getOrCreateDescriptorSetLayout(DescriptorSetLayoutKey const& key);
        vko::DescriptorSetLayout const& getDescriptorSetLayout(DescriptorSetLayoutHandle handle) const;

        PipelineLayoutHandle getOrCreatePipelineLayout(PipelineLayoutKey const& key);
        vko::PipelineLayout const& getPipelineLayout(PipelineLayoutHandle handle) const;

        PipelineHandle getOrCreatePipeline(PipelineKey const& key);
        vko::Pipeline const& getPipeline(PipelineHandle handle) const;

    private:
        void uploadBuffer(Buffer const& buffer, void const* data, std::size_t dataSize, std::size_t offset);
        void uploadBuffer(Buffer const& buffer, nstl::span<unsigned char const> bytes, std::size_t offset);
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

        ResourceContainer<Image> m_images;
        ResourceContainer<Buffer> m_buffers;
        ResourceContainer<vko::ShaderModule> m_shaderModules;
        ResourceContainer<vko::Sampler> m_samplers;

        ResourceContainer<Texture> m_textures;
        ResourceContainer<Material> m_materials;
        ResourceContainer<Mesh> m_meshes;

        nstl::vector<vko::DescriptorSetLayout> m_descriptorSetLayouts;
        nstl::unordered_map<DescriptorSetLayoutKey, DescriptorSetLayoutHandle> m_descriptorSetLayoutHandles;

        nstl::vector<vko::PipelineLayout> m_pipelineLayouts;
        nstl::unordered_map<PipelineLayoutKey, PipelineLayoutHandle> m_pipelineLayoutHandles;

        nstl::vector<vko::Pipeline> m_pipelines;
        nstl::unordered_map<PipelineKey, PipelineHandle> m_pipelineHandles;
    };
}
