#include "Queue.h"

#include "Utils.h"

vkr::Queue::Queue(VkQueue handle, QueueFamily const& queueFamily)
    : m_handle(handle)
    , m_family(queueFamily)
{

}

void vkr::Queue::waitIdle() const
{
    VKR_ASSERT(vkQueueWaitIdle(m_handle));
}
