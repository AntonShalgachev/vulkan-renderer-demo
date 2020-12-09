#pragma once

#include "framework.h"

namespace vkr
{
    class Sampler
    {
    public:
        Sampler();
        ~Sampler();

        VkSampler getHandle() const { return m_handle; }

    private:
        VkSampler m_handle;
    };
}
