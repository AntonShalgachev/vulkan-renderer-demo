#include "Application.h"
#include "Instance.h"
#include "Surface.h"
#include "PhysicalDevice.h"
#include "Device.h"

namespace vkr
{
    class ApplicationImpl
    {
    public:
        ApplicationImpl(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window);

    private:
        Instance m_instance;
        Surface m_surface;
        //PhysicalDevice m_physicalDevice;
        //Device m_device;
    };

}

vkr::ApplicationImpl::ApplicationImpl(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window)
    : m_instance(name, extensions, enableValidation, enableApiDump)
    , m_surface(m_instance, window)
{

}

// /////////////////////////////////////////////////////////////////////

vkr::Application::Application(std::string const& name, std::vector<char const*> const extensions, bool enableValidation, bool enableApiDump, Window const& window)
{
    m_impl = std::make_unique<ApplicationImpl>(name, extensions, enableValidation, enableApiDump, window);
}

//vkr::Instance const& vkr::Application::getInstance() const
//{
//
//}
//
//vkr::Surface const& vkr::Application::getSurface() const
//{
//
//}
//
//vkr::PhysicalDevice const& vkr::Application::getPhysicalDevice() const
//{
//
//}
//
//vkr::Device const& vkr::Application::getDevice() const
//{
//
//}
