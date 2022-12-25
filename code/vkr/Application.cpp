#include "Application.h"

#include "vko/Instance.h"
#include "vko/Surface.h"
#include "vko/PhysicalDevice.h"
#include "vko/Device.h"
#include "vko/Queue.h"
#include "vko/Window.h"
#include "vko/Assert.h"

#include "vkr/PhysicalDeviceSurfaceParameters.h"

#include "nstl/vector.h"

#include <assert.h>

namespace
{
    const nstl::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkSurfaceCapabilitiesKHR queryCapabilities(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        VkSurfaceCapabilitiesKHR result{};
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.getHandle(), surface.getHandle(), &result));
        return result;
    }

    nstl::vector<VkSurfaceFormatKHR> queryFormats(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        uint32_t count = 0;
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getHandle(), &count, nullptr));

        if (count <= 0)
            return {};

        nstl::vector<VkSurfaceFormatKHR> result;
        result.resize(count);

        VKO_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.getHandle(), surface.getHandle(), &count, result.data()));

        return result;
    }

    nstl::vector<VkPresentModeKHR> queryPresentModes(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        uint32_t count = 0;
        VKO_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getHandle(), &count, nullptr));

        if (count <= 0)
            return {};

        nstl::vector<VkPresentModeKHR> result;
        result.resize(count);

        VKO_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.getHandle(), surface.getHandle(), &count, result.data()));

        return result;
    }

    bool queryPresentationSupport(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface, vko::QueueFamily const& queueFamily)
    {
        VkBool32 result = false;
        VKO_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.getHandle(), queueFamily.getIndex(), surface.getHandle(), &result));
        return result;
    }

    vkr::PhysicalDeviceSurfaceParameters queryParameters(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        vkr::PhysicalDeviceSurfaceParameters params;
        params.capabilities = queryCapabilities(physicalDevice, surface);
        params.formats = queryFormats(physicalDevice, surface);
        params.presentModes = queryPresentModes(physicalDevice, surface);

        for (vko::QueueFamily const& queueFamily : physicalDevice.getQueueFamilies())
        {
            if (queueFamily.getProperties().queueFlags & VK_QUEUE_GRAPHICS_BIT)
                params.graphicsQueueFamily = &queueFamily;

            if (queryPresentationSupport(physicalDevice, surface, queueFamily))
                params.presentQueueFamily = &queueFamily;

            if (params.graphicsQueueFamily && params.presentQueueFamily)
                break;
        }

        return params;
    }

    bool isDeviceSuitable(vko::PhysicalDevice const& physicalDevice, vko::Surface const& surface)
    {
        if (!physicalDevice.areExtensionsSupported(DEVICE_EXTENSIONS))
            return false;

        if (!physicalDevice.getFeatures().samplerAnisotropy)
            return false;

        vkr::PhysicalDeviceSurfaceParameters params = queryParameters(physicalDevice, surface);
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
            , m_params(queryParameters(m_physicalDevice, m_surface))
            , m_device(getPhysicalDevice(), *m_params.graphicsQueueFamily, *m_params.presentQueueFamily, DEVICE_EXTENSIONS)
        {

        }

        vko::Instance const& getInstance() const { return m_instance; }
        vko::Surface const& getSurface() const { return m_surface; }
        vko::Device const& getDevice() const { return m_device; }

        vko::PhysicalDevice const& getPhysicalDevice() const { return m_physicalDevice; }
        PhysicalDeviceSurfaceParameters const& getParameters() const { return m_params; }

        void onSurfaceChanged()
        {
            m_params.capabilities = queryCapabilities(m_physicalDevice, m_surface);
        }

    private:
        vko::Instance m_instance;
        vko::Surface m_surface;

        vko::PhysicalDevice m_physicalDevice;
        PhysicalDeviceSurfaceParameters m_params;

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

vkr::PhysicalDeviceSurfaceParameters const& vkr::Application::getPhysicalDeviceSurfaceParameters() const
{
    return m_impl->getParameters();
}

void vkr::Application::onSurfaceChanged()
{
    return m_impl->onSurfaceChanged();
}
