#pragma once

#include "framework.h"

namespace vkr
{
    class Sampler
    {
    public:
        Sampler();
        ~Sampler();

        VkSampler getHandle() const { return m_sampler; }

    private:
        VkSampler m_sampler;
    };
}
