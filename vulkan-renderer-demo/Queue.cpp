#include "Queue.h"

vkr::Queue::Queue(VkQueue handle, QueueFamily const& queueFamily)
    : m_handle(handle)
    , m_family(queueFamily)
{

}
