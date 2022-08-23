#include "DebugMessenger.h"

#include <stdexcept>

#include "Instance.h"
#include "DebugMessage.h"

namespace
{
    vko::DebugMessage::Level convertLevel(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
    {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            return vko::DebugMessage::Level::Warning;
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            return vko::DebugMessage::Level::Error;

        return vko::DebugMessage::Level::Info;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        vko::DebugMessenger* self = static_cast<vko::DebugMessenger*>(userData);

        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            vko::DebugMessage message;
            message.level = convertLevel(severity);
            message.id = callbackData->pMessageIdName;
            message.text = callbackData->pMessage;

            self->onMessage(std::move(message));
        }

        return VK_FALSE;
    }
}

vko::DebugMessenger::DebugMessenger(Instance const& instance, std::function<void(DebugMessage)> onDebugMessage) : m_onDebugMessage(std::move(onDebugMessage)), m_instance(instance)
{
    m_createFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance.getHandle(), "vkCreateDebugUtilsMessengerEXT"));
    m_destroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance.getHandle(), "vkDestroyDebugUtilsMessengerEXT"));

    if (!m_createFunc || !m_destroyFunc)
        throw std::runtime_error("Failed to create a debug messenger");

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugMessageCallback;
    createInfo.pUserData = this;

    if (m_createFunc(m_instance.getHandle(), &createInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a debug messenger");
}

vko::DebugMessenger::~DebugMessenger()
{
    m_destroyFunc(m_instance.getHandle(), m_handle, nullptr);
}

void vko::DebugMessenger::onMessage(DebugMessage message)
{
    m_onDebugMessage(std::move(message));
}
