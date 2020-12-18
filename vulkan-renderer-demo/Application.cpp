#include "Application.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "PhysicalDeviceSurfaceContainer.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"

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
}

namespace vkr
{
    class ApplicationImpl
    {
    public:
        ApplicationImpl(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window);

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
        Surface m_surface;
        std::vector<vkr::PhysicalDeviceSurfaceContainer> m_physicalDevices;
        std::size_t m_currentPhysicalDeviceIndex;
        Device m_device;
    };

}

vkr::ApplicationImpl::ApplicationImpl(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window)
    : m_instance(name, extensions, enableValidation, enableApiDump)
    , m_surface(m_instance, window)
    , m_physicalDevices(m_instance.findPhysicalDevices(m_surface))
    , m_currentPhysicalDeviceIndex(findSuitablePhysicalDeviceIndex(m_physicalDevices))
    , m_device(getPhysicalDeviceSurfaceContainer(), DEVICE_EXTENSIONS)
{

}

// /////////////////////////////////////////////////////////////////////

vkr::Application::Application(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window)
{
    m_impl = std::make_unique<ApplicationImpl>(name, extensions, enableValidation, enableApiDump, window);
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
