#pragma once

#include <vulkan/vulkan.h>
#include "Object.h"
#include <vector>
#include "UniqueHandle.h"

namespace vkr
{
    class DeviceMemory : public Object
    {
    public:
        explicit DeviceMemory(Application const& app, VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags memoryProperties);
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
        UniqueHandle<VkDeviceMemory> m_handle;
    };
}
