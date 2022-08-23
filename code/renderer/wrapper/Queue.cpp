#include "Queue.h"

#include "Utils.h"

vko::Queue::Queue(VkQueue handle, QueueFamily const& queueFamily)
    : m_handle(handle)
    , m_family(queueFamily)
{

}

void vko::Queue::waitIdle() const
{
    VKR_ASSERT(vkQueueWaitIdle(m_handle));
}
