#include "Semaphore.h"
#include "Device.h"

vkr::Semaphore::Semaphore(Application const& app) : Object(app)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(getDevice().getHandle(), &semaphoreCreateInfo, nullptr, &m_handle.get()) != VK_SUCCESS)
        throw std::runtime_error("failed to create semaphore!");
}

vkr::Semaphore::~Semaphore()
{
    vkDestroySemaphore(getDevice().getHandle(), m_handle, nullptr);
}
