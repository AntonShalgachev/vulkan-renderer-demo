#include "Queue.h"

#include "vko/Assert.h"

vko::Queue::Queue(VkQueue handle, QueueFamily const& queueFamily)
    : m_handle(handle)
    , m_family(queueFamily)
{

}

void vko::Queue::waitIdle() const
{
    VKO_VERIFY(vkQueueWaitIdle(m_handle));
}
