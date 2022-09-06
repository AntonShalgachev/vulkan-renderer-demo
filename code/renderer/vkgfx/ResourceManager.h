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
    enum class BufferUsage;

    struct ShaderModule;
    struct ShaderModuleHandle;

    struct Sampler;
    struct SamplerHandle;

    struct Texture;
    struct TextureHandle;

    struct Material;
    struct MaterialHandle;

    struct Mesh;
    struct MeshHandle;

    struct Pipeline;
    struct PipelineHandle;
    struct PipelineKey;

    class ResourceManager
    {
    public:
        ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue);
        ~ResourceManager();

        ImageHandle createImage(ImageMetadata metadata);
        void uploadImage(ImageHandle handle, void const* data, std::size_t dataSize);
        void uploadImage(ImageHandle handle, std::span<unsigned char const> bytes);

        BufferHandle createBuffer(std::size_t size, BufferUsage usage);
        void uploadBuffer(BufferHandle handle, void const* data, std::size_t dataSize, std::size_t offset = 0);
        void uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes, std::size_t offset = 0);

        ShaderModuleHandle createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint = "main");

        SamplerHandle createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV);

        TextureHandle createTexture(Texture texture);
        MaterialHandle createMaterial(Material material);
        MeshHandle createMesh(Mesh mesh);

        PipelineHandle getOrCreatePipeline(PipelineKey const& key);

    private:
        vko::Device const& m_device;
        vko::PhysicalDevice const& m_physicalDevice; // TODO replace with the allocator
        vko::CommandPool const& m_uploadCommandPool;
        vko::Queue const& m_uploadQueue;

        std::vector<Image> m_images;
        std::vector<Buffer> m_buffers;
        std::vector<ShaderModule> m_shaderModules;
        std::vector<Sampler> m_samplers;

        std::vector<Texture> m_textures;
        std::vector<Material> m_materials;
        std::vector<Mesh> m_meshes;

        std::vector<Pipeline> m_pipelines;
        std::map<PipelineKey, PipelineHandle> m_pipelineHandles;
    };
}
