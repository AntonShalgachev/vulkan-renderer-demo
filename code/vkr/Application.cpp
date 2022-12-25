#include "Application.h"

#include "vko/Instance.h"
#include "vko/Surface.h"
#include "vko/PhysicalDevice.h"
#include "vko/Device.h"
#include "vko/Queue.h"
#include "vko/Window.h"
#include "vko/Assert.h"
#include "vko/PhysicalDeviceSurfaceParameters.h"

#include "nstl/vector.h"

#include <assert.h>

namespace
{
    const nstl::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    bool isDeviceSuitable(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        if (!physicalDevice.areExtensionsSupported(DEVICE_EXTENSIONS))
            return false;

        if (!physicalDevice.getFeatures().samplerAnisotropy)
            return false;

        vko::PhysicalDeviceSurfaceParameters params = vko::queryPhysicalDeviceSurfaceParameters(physicalDevice, surface);
        return params.graphicsQueueFamily && params.presentQueueFamily && !params.formats.empty() && !params.presentModes.empty();
    }

    nstl::vector<const char*> createInstanceExtensions(bool enableValidation, vko::Window const& window)
    {
        nstl::vector<const char*> extensions = window.getRequiredInstanceExtensions();
        
        if (enableValidation)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    vko::PhysicalDevice findPhysicalDevice(vko::Instance const& instance, vko::Surface const& surface)
    {
        nstl::vector<vko::PhysicalDevice> physicalDevices = instance.findPhysicalDevices();
        
        for (vko::PhysicalDevice& physicalDevice : physicalDevices)
            if (isDeviceSuitable(physicalDevice, surface))
                return nstl::move(physicalDevice);

        assert(false);
        return nstl::move(physicalDevices[0]);
    }
}

namespace vkr
{
    class ApplicationImpl
    {
    public:
        ApplicationImpl(char const* name, bool enableValidation, bool enableApiDump, vko::Window const& window, nstl::function<void(vko::DebugMessage)> onDebugMessage)
            : m_instance(name, createInstanceExtensions(enableValidation, window), enableValidation, enableApiDump, enableValidation ? nstl::move(onDebugMessage) : nullptr)
            , m_surface(window.createSurface(m_instance))
            , m_physicalDevice(findPhysicalDevice(m_instance, m_surface))
            , m_params(vko::queryPhysicalDeviceSurfaceParameters(m_physicalDevice, m_surface))
            , m_device(getPhysicalDevice(), *m_params.graphicsQueueFamily, *m_params.presentQueueFamily, DEVICE_EXTENSIONS)
        {

        }

        vko::Instance const& getInstance() const { return m_instance; }
        vko::Surface const& getSurface() const { return m_surface; }
        vko::Device const& getDevice() const { return m_device; }

        vko::PhysicalDevice const& getPhysicalDevice() const { return m_physicalDevice; }
        vko::PhysicalDeviceSurfaceParameters const& getParameters() const { return m_params; }

        void onSurfaceChanged()
        {
            m_params = vko::queryPhysicalDeviceSurfaceParameters(m_physicalDevice, m_surface);
        }

    private:
        vko::Instance m_instance;
        vko::Surface m_surface;

        vko::PhysicalDevice m_physicalDevice;
        vko::PhysicalDeviceSurfaceParameters m_params;

        vko::Device m_device;
    };
}

vkr::Application::Application(char const* name, bool enableValidation, bool enableApiDump, vko::Window const& window, nstl::function<void(vko::DebugMessage)> onDebugMessage)
{
    m_impl = nstl::make_unique<ApplicationImpl>(name, enableValidation, enableApiDump, window, nstl::move(onDebugMessage));
}

vkr::Application::~Application() = default;

vko::Instance const& vkr::Application::getInstance() const
{
    return m_impl->getInstance();
}

vko::Surface const& vkr::Application::getSurface() const
{
    return m_impl->getSurface();
}

vko::Device const& vkr::Application::getDevice() const
{
    return m_impl->getDevice();
}

vko::PhysicalDevice const& vkr::Application::getPhysicalDevice() const
{
    return m_impl->getPhysicalDevice();
}

vko::PhysicalDeviceSurfaceParameters const& vkr::Application::getPhysicalDeviceSurfaceParameters() const
{
    return m_impl->getParameters();
}

void vkr::Application::onSurfaceChanged()
{
    return m_impl->onSurfaceChanged();
}
