#pragma once

#include "framework.h"

namespace vkr
{
    class DeviceMemory
    {
    public:
        explicit DeviceMemory(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties);
        ~DeviceMemory();

        DeviceMemory(DeviceMemory const&) = delete;
        DeviceMemory(DeviceMemory&&) = delete;
        DeviceMemory& operator=(DeviceMemory const&) = delete;
        DeviceMemory& operator=(DeviceMemory&&) = delete;

        void copyFrom(void const* sourcePointer, std::size_t sourceSize);

        template<typename T>
        void copyFrom(std::vector<T> const& source)
        {
            copyFrom(source.data(), sizeof(T) * source.size());
        }

        VkDeviceMemory getHandle() const { return m_handle; };

    private:
        VkDeviceMemory m_handle = VK_NULL_HANDLE;
    };
}
