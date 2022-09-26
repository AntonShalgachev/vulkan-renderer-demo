#include "Application.h"
#include "wrapper/Instance.h"
#include "wrapper/Surface.h"
#include "wrapper/PhysicalDevice.h"
#include "wrapper/Device.h"
#include "PhysicalDeviceSurfaceContainer.h"
#include "PhysicalDeviceSurfaceParameters.h"
#include "QueueFamilyIndices.h"
#include "wrapper/CommandPool.h"
#include <stdexcept>
#include "wrapper/Queue.h"
#include "wrapper/Window.h"

namespace
{
    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    std::vector<vkr::PhysicalDeviceSurfaceContainer> createPhysicalDeviceSurfaceContainer(std::vector<vko::PhysicalDevice> devices, vko::Surface const& surface)
    {
        std::vector<vkr::PhysicalDeviceSurfaceContainer> result;
        result.reserve(devices.size());

        for (vko::PhysicalDevice& device : devices)
            result.emplace_back(std::move(device), surface);

        return result;
    }

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

    std::vector<const char*> createInstanceExtensions(bool enableValidation, vko::Window const& window)
    {
        std::vector<const char*> extensions = window.getRequiredInstanceExtensions();
        
        if (enableValidation)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
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
        ApplicationImpl(std::string const& name, bool enableValidation, bool enableApiDump, vko::Window const& window, std::function<void(vko::DebugMessage)> onDebugMessage)
            : m_instance(name, createInstanceExtensions(enableValidation, window), enableValidation, enableApiDump, enableValidation ? std::move(onDebugMessage) : nullptr)
            , m_surface(window.createSurface(m_instance))
            , m_physicalDevices(createPhysicalDeviceSurfaceContainer(m_instance.findPhysicalDevices(), m_surface))
            , m_currentPhysicalDeviceIndex(findSuitablePhysicalDeviceIndex(m_physicalDevices))
            , m_device(getPhysicalDevice(), getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily(), getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getPresentQueueFamily(), DEVICE_EXTENSIONS)
        {

        }

        vko::Instance const& getInstance() const { return m_instance; }
        vko::Surface const& getSurface() const { return m_surface; }
        vko::Device const& getDevice() const { return m_device; }

        PhysicalDeviceSurfaceContainer const& getPhysicalDeviceSurfaceContainer() const { return m_physicalDevices[m_currentPhysicalDeviceIndex]; }
        PhysicalDeviceSurfaceContainer& getPhysicalDeviceSurfaceContainer() { return m_physicalDevices[m_currentPhysicalDeviceIndex]; }
        vko::PhysicalDevice const& getPhysicalDevice() const { return getPhysicalDeviceSurfaceContainer().getPhysicalDevice(); }
        PhysicalDeviceSurfaceParameters const& getPhysicalDeviceSurfaceParameters() const { return getPhysicalDeviceSurfaceContainer().getParameters(); }
        PhysicalDeviceSurfaceParameters& getPhysicalDeviceSurfaceParameters() { return getPhysicalDeviceSurfaceContainer().getParameters(); }

    private:
        vko::Instance m_instance;
        vko::Surface m_surface;
        std::vector<vkr::PhysicalDeviceSurfaceContainer> m_physicalDevices;
        std::size_t m_currentPhysicalDeviceIndex;
        vko::Device m_device;
    };
}

vkr::Application::Application(std::string const& name, bool enableValidation, bool enableApiDump, vko::Window const& window, std::function<void(vko::DebugMessage)> onDebugMessage)
{
    m_impl = std::make_unique<ApplicationImpl>(name, enableValidation, enableApiDump, window, std::move(onDebugMessage));

    setDebugName(getDevice().getGraphicsQueue().getHandle(), "GraphicsQueue");
    setDebugName(getDevice().getPresentQueue().getHandle(), "PresentQueue");

    m_shortLivedCommandPool = std::make_unique<vko::CommandPool>(getDevice(), getPhysicalDeviceSurfaceParameters().getQueueFamilyIndices().getGraphicsQueueFamily());
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

vko::CommandPool const& vkr::Application::getShortLivedCommandPool() const
{
    return *m_shortLivedCommandPool;
}

vkr::PhysicalDeviceSurfaceParameters const& vkr::Application::getPhysicalDeviceSurfaceParameters() const
{
    return m_impl->getPhysicalDeviceSurfaceParameters();
}

vko::PhysicalDevice const& vkr::Application::getPhysicalDevice() const
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
