#include "Application.h"
#include "wrapper/Instance.h"
#include "wrapper/Surface.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Device.h"
#include "PhysicalDeviceSurfaceContainer.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "wrapper/CommandPool.h"
#include "Window.h"
#include "wrapper/DebugMessenger.h"
#include <stdexcept>
#include "wrapper/Queue.h"

namespace
{
    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    bool isDeviceSuitable(vkr::PhysicalDeviceSurfaceContainer const& container)
    {
        auto const& physicalDevice = container.getPhysicalDevice();
        auto const& parameters = container.getParameters();

        bool const areExtensionsSupported = physicalDevice.areExtensionsSupported(DEVICE_EXTENSIONS);

        bool swapchainSupported = false;
        if (areExtensionsSupported)
        {
            swapchainSupported = !parameters.getFormats().empty() && !parameters.getPresentModes().empty();
        }

        return parameters.getQueueFamilyIndices().isValid() && areExtensionsSupported && swapchainSupported && physicalDevice.getFeatures().samplerAnisotropy;
    }

    std::size_t findSuitablePhysicalDeviceIndex(std::vector<vkr::PhysicalDeviceSurfaceContainer> const& physicalDevices)
    {
        for (std::size_t index = 0; index < physicalDevices.size(); index++)
        {
            auto const& physicalDevice = physicalDevices[index];

            if (isDeviceSuitable(physicalDevice))
                return index;
        }

        throw std::runtime_error("failed to find a suitable GPU!");
    }

    std::vector<const char*> createInstanceExtensions(bool enableValidation, vkr::Window const& window)
    {
        std::vector<const char*> extensions = window.getRequiredInstanceExtensions();
        
        if (enableValidation)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    std::unique_ptr<vkr::DebugMessenger> createDebugMessenger(vkr::Instance& instance, bool enableValidation, std::function<void(vkr::DebugMessage)> onDebugMessage)
    {
        if (!enableValidation)
            return nullptr;

        return std::make_unique<vkr::DebugMessenger>(instance, std::move(onDebugMessage));
    }

    template<typename T>
    void setDebugName(vkr::Application const& app, T handle, VkObjectType type, char const* name)
    {
        VkInstance instance = app.getInstance().getHandle();
        VkDevice device = app.getDevice().getHandle();

        // TODO cache function pointer
		auto pfnSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));

        if (pfnSetDebugUtilsObjectNameEXT)
        {
            VkDebugUtilsObjectNameInfoEXT nameInfo{};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = type;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(handle);
            nameInfo.pObjectName = name;

            VKR_ASSERT(pfnSetDebugUtilsObjectNameEXT(device, &nameInfo));
        }
    }
}

namespace vkr
{
    class ApplicationImpl
    {
    public:

        ApplicationImpl(std::string const& name, bool enableValidation, bool enableApiDump, Window const& window, std::function<void(DebugMessage)> onDebugMessage)
            : m_instance(name, createInstanceExtensions(enableValidation, window), enableValidation, enableApiDump)
            , m_debugMessenger(createDebugMessenger(m_instance, enableValidation, std::move(onDebugMessage)))
            , m_surface(m_instance, window)
            , m_physicalDevices(m_instance.findPhysicalDevices(m_surface))
            , m_currentPhysicalDeviceIndex(findSuitablePhysicalDeviceIndex(m_physicalDevices))
            , m_device(getPhysicalDeviceSurfaceContainer(), DEVICE_EXTENSIONS)
        {

        }

        Instance const& getInstance() const { return m_instance; }
        Surface const& getSurface() const { return m_surface; }
        Device const& getDevice() const { return m_device; }

        PhysicalDeviceSurfaceContainer const& getPhysicalDeviceSurfaceContainer() const { return m_physicalDevices[m_currentPhysicalDeviceIndex]; }
        PhysicalDeviceSurfaceContainer& getPhysicalDeviceSurfaceContainer() { return m_physicalDevices[m_currentPhysicalDeviceIndex]; }
        PhysicalDevice const& getPhysicalDevice() const { return getPhysicalDeviceSurfaceContainer().getPhysicalDevice(); }
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const { return getPhysicalDeviceSurfaceContainer().getParameters(); }
        PhysicalDeviceSurfaceParameters& getPhysicalDeviceSurfaceParameters() { return getPhysicalDeviceSurfaceContainer().getParameters(); }

    private:
        Instance m_instance;
        std::unique_ptr<vkr::DebugMessenger> m_debugMessenger;
        Surface m_surface;
        std::vector<vkr::PhysicalDeviceSurfaceContainer> m_physicalDevices;
        std::size_t m_currentPhysicalDeviceIndex;
        Device m_device;
    };
}

vkr::Application::Application(std::string const& name, bool enableValidation, bool enableApiDump, Window const& window, std::function<void(DebugMessage)> onDebugMessage)
{
    m_impl = std::make_unique<ApplicationImpl>(name, enableValidation, enableApiDump, window, std::move(onDebugMessage));

    setDebugName(getDevice().getGraphicsQueue().getHandle(), "GraphicsQueue");
    setDebugName(getDevice().getPresentQueue().getHandle(), "PresentQueue");

    m_shortLivedCommandPool = std::make_unique<CommandPool>(getDevice(), getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily());
}

vkr::Application::~Application() = default;

vkr::Instance const& vkr::Application::getInstance() const
{
    return m_impl->getInstance();
}

vkr::Surface const& vkr::Application::getSurface() const
{
    return m_impl->getSurface();
}

vkr::Device const& vkr::Application::getDevice() const
{
    return m_impl->getDevice();
}

vkr::CommandPool const& vkr::Application::getShortLivedCommandPool() const
{
    return *m_shortLivedCommandPool;
}

vkr::PhysicalDeviceSurfaceParameters const& vkr::Application::getPhysicalDeviceSurfaceParameters() const
{
    return m_impl->getPhysicalDeviceSurfaceParameters();
}

vkr::PhysicalDevice const& vkr::Application::getPhysicalDevice() const
{
    return m_impl->getPhysicalDevice();
}

void vkr::Application::onSurfaceChanged()
{
    m_impl->getPhysicalDeviceSurfaceParameters().onSurfaceChanged();
}

void vkr::Application::setDebugName(VkQueue handle, char const* name) const
{
    ::setDebugName(*this, handle, VK_OBJECT_TYPE_QUEUE, name);
}

void vkr::Application::setDebugName(VkInstance handle, char const* name) const
{
    ::setDebugName(*this, handle, VK_OBJECT_TYPE_INSTANCE, name);
}

void vkr::Application::setDebugName(VkSemaphore handle, char const* name) const
{
    ::setDebugName(*this, handle, VK_OBJECT_TYPE_SEMAPHORE, name);
}
