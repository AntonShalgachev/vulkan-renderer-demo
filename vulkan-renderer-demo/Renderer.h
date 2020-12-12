#pragma once

#include "framework.h"
#include "Instance.h"
#include "Surface.h"

namespace vkr
{
    class PhysicalDevice;
    class Device;
    class CommandPool;

    class Renderer
    {
    public:
        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool IsComplete()
            {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct PhysicalDeviceProperties
        {
            QueueFamilyIndices queueFamilyIndices;
            SwapchainSupportDetails swapchainSupportDetails;
        };

    public:
        Renderer(GLFWwindow* window);
        ~Renderer();

        void OnSurfaceChanged(int width, int height);

        VkSurfaceKHR getSurfaceHandle() const { return m_surface.getHandle(); }
        VkPhysicalDevice getPhysicalDevice() const;
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkCommandPool getCommandPool() const;

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        PhysicalDeviceProperties const& getPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }

        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        Instance m_instance;
        Surface m_surface;
        std::shared_ptr<PhysicalDevice> m_physicalDevice;
        std::unique_ptr<Device> m_device;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        std::unique_ptr<CommandPool> m_commandPool;

        PhysicalDeviceProperties m_physicalDeviceProperties;

        int m_width = -1;
        int m_height = -1;
    };
}
