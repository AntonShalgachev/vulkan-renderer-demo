#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "UniqueHandle.h"

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

        void copyFrom(void const* sourcePointer, std::size_t sourceSize) const;

        template<typename T>
        void copyFrom(std::vector<T> const& source) const
        {
            copyFrom(source.data(), sizeof(T) * source.size());
        }

        VkDeviceMemory getHandle() const { return m_handle; };

    private:
        Device const& m_device;
        UniqueHandle<VkDeviceMemory> m_handle;
    };
}
