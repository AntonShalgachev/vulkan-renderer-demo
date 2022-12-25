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
    struct ApplicationImpl
    {
    public:
        ApplicationImpl(char const* name, bool enableValidation, bool enableApiDump, vko::Window const& window, nstl::function<void(vko::DebugMessage)> onDebugMessage)
            : instance(name, createInstanceExtensions(enableValidation, window), enableValidation, enableApiDump, enableValidation ? nstl::move(onDebugMessage) : nullptr)
            , surface(window.createSurface(instance))
            , physicalDevice(findPhysicalDevice(instance, surface))
            , params(vko::queryPhysicalDeviceSurfaceParameters(physicalDevice, surface))
            , device(physicalDevice, *params.graphicsQueueFamily, *params.presentQueueFamily, DEVICE_EXTENSIONS)
        {

        }

        void updatePhysicalDeviceSurfaceParameters()
        {
            params = vko::queryPhysicalDeviceSurfaceParameters(physicalDevice, surface);
        }

        vko::Instance instance;
        vko::Surface surface;

        vko::PhysicalDevice physicalDevice;
        vko::PhysicalDeviceSurfaceParameters params;

        vko::Device device;
    };
}

vkr::Application::Application(char const* name, bool enableValidation, bool enableApiDump, vko::Window const& window, nstl::function<void(vko::DebugMessage)> onDebugMessage)
{
    m_impl = nstl::make_unique<ApplicationImpl>(name, enableValidation, enableApiDump, window, nstl::move(onDebugMessage));
}

vkr::Application::~Application() = default;

vko::Instance const& vkr::Application::getInstance() const
{
    return m_impl->instance;
}

vko::Surface const& vkr::Application::getSurface() const
{
    return m_impl->surface;
}

vko::Device const& vkr::Application::getDevice() const
{
    return m_impl->device;
}

vko::PhysicalDevice const& vkr::Application::getPhysicalDevice() const
{
    return m_impl->physicalDevice;
}

vko::PhysicalDeviceSurfaceParameters const& vkr::Application::getPhysicalDeviceSurfaceParameters() const
{
    return m_impl->params;
}

void vkr::Application::onSurfaceChanged()
{
    return m_impl->updatePhysicalDeviceSurfaceParameters();
}
