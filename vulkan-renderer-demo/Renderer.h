#pragma once

#include "framework.h"
#include "Instance.h"
#include "Surface.h"

namespace vkr
{
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
        VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
        VkDevice getDevice() const { return m_device; }
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkCommandPool getCommandPool() const { return m_commandPool; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

        PhysicalDeviceProperties const& getPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }

        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        Instance m_instance;
        Surface m_surface;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;

        PhysicalDeviceProperties m_physicalDeviceProperties;

        int m_width = -1;
        int m_height = -1;
    };
}
