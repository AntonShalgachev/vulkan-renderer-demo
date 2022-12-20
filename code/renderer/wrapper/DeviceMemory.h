#pragma once
#include "renderer/wrapper/UniqueHandle.h"

#include "nstl/vector.h"

#include <vulkan/vulkan.h>

namespace vko
{
    class Device;
    class PhysicalDevice;

    // TODO use span?
    class DeviceMemory
    {
    public:
        explicit DeviceMemory(Device const& device, PhysicalDevice const& physicalDevice, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties);
        ~DeviceMemory();

        DeviceMemory(DeviceMemory const&) = default;
        DeviceMemory(DeviceMemory&&) = default;
        DeviceMemory& operator=(DeviceMemory const&) = default;
        DeviceMemory& operator=(DeviceMemory&&) = default;

        void copyFrom(void const* sourcePointer, std::size_t sourceSize, std::size_t offset = 0) const;

        template<typename T>
        void copyFrom(nstl::vector<T> const& source, std::size_t offset = 0) const
        {
            copyFrom(source.data(), sizeof(T) * source.size(), offset);
        }

        VkDeviceMemory getHandle() const { return m_handle; };

        VkMemoryRequirements const& getRequirements() const { return m_requirements; }
        VkMemoryPropertyFlags const& getProperties() const { return m_properties; }

    private:
        VkDevice m_device;
        UniqueHandle<VkDeviceMemory> m_handle;
        void* m_data = nullptr;

        VkMemoryRequirements m_requirements{};
        VkMemoryPropertyFlags m_properties{};
    };
}
