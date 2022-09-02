#pragma once

#include <span>
#include <vector>
#include <string>

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

    struct ShaderModule;
    struct ShaderModuleHandle;

    struct Sampler;
    struct SamplerHandle;

    class ResourceManager
    {
    public:
        ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue);
        ~ResourceManager();

        ImageHandle createImage(ImageMetadata metadata);
        void uploadImage(ImageHandle handle, std::span<unsigned char const> bytes);

        BufferHandle createBuffer(std::size_t size);
        void uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes);

        ShaderModuleHandle createShaderModule(std::span<unsigned char const> bytes, vko::ShaderModuleType type, std::string entryPoint = "main");

        SamplerHandle createSampler(vko::SamplerFilterMode magFilter, vko::SamplerFilterMode minFilter, vko::SamplerWrapMode wrapU, vko::SamplerWrapMode wrapV);

    private:
        vko::Device const& m_device;
        vko::PhysicalDevice const& m_physicalDevice; // TODO replace with the allocator
        vko::CommandPool const& m_uploadCommandPool;
        vko::Queue const& m_uploadQueue;

        std::vector<Image> m_images;
        std::vector<Buffer> m_buffers;
        std::vector<ShaderModule> m_shaderModules;
        std::vector<Sampler> m_samplers;
    };
}
