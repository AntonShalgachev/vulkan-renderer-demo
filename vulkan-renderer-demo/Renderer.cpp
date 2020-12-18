#include "Renderer.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "CommandPool.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "PhysicalDeviceSurfaceContainer.h"
#include "Window.h"
#include "Application.h"

namespace
{
    bool const VALIDATION_ENABLED = true;
    bool const API_DUMP_ENABLED = false;
}

namespace vkr
{
    Renderer::Renderer(Window const& window)
        : m_application(std::make_unique<Application>("Vulkan demo", window.getRequiredInstanceExtensions(), VALIDATION_ENABLED, API_DUMP_ENABLED, window))
    {
        createCommandPool();
    }

    Renderer::~Renderer() = default;

    void Renderer::onSurfaceChanged()
    {
        m_application->onSurfaceChanged();
    }

    VkSurfaceKHR Renderer::getSurfaceHandle() const
    {
        return m_application->getSurface().getHandle();
    }

    VkDevice Renderer::getDevice() const
    {
        return m_application->getDevice().getHandle();
    }

    VkCommandPool Renderer::getCommandPool() const
    {
        return m_commandPool->getHandle();
    }

    vkr::PhysicalDevice const& Renderer::getPhysicalDevice() const
    {
        return m_application->getPhysicalDevice();
    }

    vkr::PhysicalDeviceSurfaceParameters const& Renderer::getPhysicalDeviceSurfaceParameters() const
    {
        return m_application->getPhysicalDeviceSurfaceParameters();
    }

    vkr::QueueFamilyIndices const& Renderer::getQueueFamilyIndices() const
    {
        return getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices();
    }

    int Renderer::getWidth() const
    {
        return m_application->getSurface().getWidth();
    }

    int Renderer::getHeight() const
    {
        return m_application->getSurface().getHeight();
    }

    void Renderer::createCommandPool()
    {
        m_commandPool = std::make_unique<CommandPool>(m_application->getDevice(), getQueueFamilyIndices().getGraphicsQueueFamily());
    }
}
