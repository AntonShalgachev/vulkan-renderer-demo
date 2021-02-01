#include "Queue.h"

vkr::Queue::Queue(VkQueue handle, QueueFamily const& queueFamily)
    : m_handle(handle)
    , m_family(queueFamily)
{

}

void vkr::Queue::waitIdle() const
{
    vkQueueWaitIdle(m_handle);
}
