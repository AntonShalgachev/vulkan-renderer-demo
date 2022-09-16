#pragma once

#include <span>
#include <vector>
#include <string>
#include <map>

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
        ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue, vko::RenderPass const& renderPass, std::size_t width, std::size_t height);
        ~ResourceManager();

        ImageHandle createImage(ImageMetadata metadata);
        void uploadImage(ImageHandle handle, void const* data, std::size_t dataSize);
        void uploadImage(ImageHandle handle, std::span<unsigned char const> bytes);
        Image const& getImage(ImageHandle handle);

        BufferHandle createBuffer(std::size_t size, BufferMetadata metadata);
        void uploadBuffer(BufferHandle handle, void const* data, std::size_t dataSize, std::size_t offset = 0);
        void uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes, std::size_t offset = 0);
        Buffer const& getBuffer(BufferHandle handle);

        ShaderModuleHandle createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint = "main");

        SamplerHandle createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV);
        Sampler const& getSampler(SamplerHandle handle);

        TextureHandle createTexture(Texture texture);
        Texture const& getTexture(TextureHandle handle);
        MaterialHandle createMaterial(Material material);
        Material const& getMaterial(MaterialHandle handle);
        MeshHandle createMesh(Mesh mesh);
        Mesh const& getMesh(MeshHandle handle);

        PipelineHandle getOrCreatePipeline(PipelineKey const& key);
        vko::Pipeline const& getPipeline(PipelineHandle handle);

    private:
        DescriptorSetLayoutHandle getOrCreateDescriptorSetLayout(DescriptorSetLayoutKey const& key);
        DescriptorSetLayoutHandle createDescriptorSetLayout(DescriptorSetLayoutKey const& key);

        PipelineLayoutHandle getOrCreatePipelineLayout(PipelineLayoutKey const& key);
        PipelineLayoutHandle createPipelineLayout(PipelineLayoutKey const& key);

        PipelineHandle createPipeline(PipelineKey const& key);

    private:
        vko::Device const& m_device;
        vko::PhysicalDevice const& m_physicalDevice; // TODO replace with the allocator
        vko::CommandPool const& m_uploadCommandPool;
        vko::Queue const& m_uploadQueue;
        vko::RenderPass const& m_renderPass; // TODO remove
        std::size_t m_width = 0; // TODO remove
        std::size_t m_height = 0; // TODO remove

        std::vector<Image> m_images;
        std::vector<Buffer> m_buffers;
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
