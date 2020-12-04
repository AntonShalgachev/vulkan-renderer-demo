#pragma once

#include "framework.h"

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

        void OnResize();

        VkInstance getInstanceHandle() const { return m_instance; }
        VkSurfaceKHR getSurfaceHandle() const { return m_surface; }
        VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
        VkDevice getDevice() const { return m_device; }
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }

        PhysicalDeviceProperties const& getPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }

        void createVulkanInstance();
        void createSurface(GLFWwindow* window);
        void pickPhysicalDevice();
        void createLogicalDevice();

        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) const;

        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;

        PhysicalDeviceProperties m_physicalDeviceProperties;
    };
}
