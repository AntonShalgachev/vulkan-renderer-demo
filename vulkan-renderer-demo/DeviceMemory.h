#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <vector>

namespace vkr
{
    class DeviceMemory : public Object
    {
    public:
        explicit DeviceMemory(Application const& app, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties);
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
