#pragma once

#include <span>
#include <vector>

namespace vko
{
    class Device;
    class PhysicalDevice;
    class CommandPool;
    class Queue;
}

namespace vkgfx
{
    struct Image;
    struct ImageHandle;
    struct ImageMetadata;

    struct Buffer;
    struct BufferHandle;

    class ResourceManager
    {
    public:
        ResourceManager(vko::Device const& device, vko::PhysicalDevice const& physicalDevice, vko::CommandPool const& uploadCommandPool, vko::Queue const& uploadQueue);
        ~ResourceManager();

        ImageHandle createImage(ImageMetadata metadata);
        void uploadImage(ImageHandle handle, std::span<unsigned char const> bytes);

        BufferHandle createBuffer(std::size_t size);
        void uploadBuffer(BufferHandle handle, std::span<unsigned char const> bytes);

    private:
        vko::Device const& m_device;
        vko::PhysicalDevice const& m_physicalDevice; // TODO replace with the allocator
        vko::CommandPool const& m_uploadCommandPool;
        vko::Queue const& m_uploadQueue;

        std::vector<Image> m_images;
        std::vector<Buffer> m_buffers;
    };
}
