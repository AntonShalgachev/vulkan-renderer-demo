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

    VkCommandPool Renderer::getCommandPool() const
    {
        return m_commandPool->getHandle();
    }

    void Renderer::createCommandPool()
    {
        QueueFamilyIndices indices =  m_application->getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices();
        m_commandPool = std::make_unique<CommandPool>(m_application->getDevice(), indices.getGraphicsQueueFamily());
    }
}
