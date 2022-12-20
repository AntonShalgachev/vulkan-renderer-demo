#include "Queue.h"

#include "renderer/wrapper/Assert.h"

vko::Queue::Queue(VkQueue handle, QueueFamily const& queueFamily)
    : m_handle(handle)
    , m_family(queueFamily)
{

}

void vko::Queue::waitIdle() const
{
    VKO_ASSERT(vkQueueWaitIdle(m_handle));
}
