#pragma once

#include <vulkan/vulkan.h>
#include <memory>

// TODO move somewhere
#include <string>
#include <stdexcept>
#define VKR_ASSERT(cmd) do { if (auto res = (cmd); res != VK_SUCCESS) throw std::runtime_error(std::to_string(res)); } while(0)

namespace vko
{
    class DeviceMemory;
    class Image;
}

namespace vkr
{
    class Application;

    namespace utils
    {
        void createImage(vkr::Application const& app, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, std::unique_ptr<vko::Image>& image, std::unique_ptr<vko::DeviceMemory>& imageMemory);

        // TODO extract somewhere else

        template<typename StringVector1, typename StringVector2>
        bool hasEveryOption(StringVector1 const& availableOptions, StringVector2 const& requestedOptions)
        {
            for (const auto& requestedOption : requestedOptions)
            {
                auto it = std::find_if(availableOptions.begin(), availableOptions.end(), [requestedOption](auto const& availableOption)
                {
                    return std::string_view{ availableOption } == std::string_view{ requestedOption };
                });

                if (it == availableOptions.end())
                    return false;
            }

            return true;
        }
    }
}
