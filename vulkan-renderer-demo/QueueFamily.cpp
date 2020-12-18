#include "QueueFamily.h"

vkr::QueueFamily::QueueFamily(uint32_t index, VkQueueFamilyProperties properties)
    : m_index(index)
    , m_properties(properties)
{

}
