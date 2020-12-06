#include "Semaphore.h"

vkr::Semaphore::Semaphore()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(temp::getDevice(), &semaphoreCreateInfo, nullptr, &m_handle) != VK_SUCCESS)
        throw std::runtime_error("failed to create semaphore!");
}

vkr::Semaphore::~Semaphore()
{
    vkDestroySemaphore(temp::getDevice(), m_handle, nullptr);
}
