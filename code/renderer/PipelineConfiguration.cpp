#include "PipelineConfiguration.h"

bool operator==(VkExtent2D const& lhs, VkExtent2D const& rhs)
{
    return lhs.width == rhs.width && lhs.height == rhs.height;
}
