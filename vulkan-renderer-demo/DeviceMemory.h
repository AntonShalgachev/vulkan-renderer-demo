#pragma once

#include "framework.h"

namespace vkr
{
    class DeviceMemory
    {
    public:
        DeviceMemory(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties);
        ~DeviceMemory();

        void copy(void const* sourcePointer, std::size_t sourceSize);

        template<typename T>
        void copy(std::vector<T> const& source)
        {
            copy(source.data(), sizeof(T) * source.size());
        }

        VkDeviceMemory getHandle() const { return m_memory; };

    private:
        VkDeviceMemory m_memory;
    };
}
