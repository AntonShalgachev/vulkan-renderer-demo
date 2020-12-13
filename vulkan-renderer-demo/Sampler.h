#pragma once

#include "framework.h"

namespace vkr
{
    class Sampler
    {
    public:
        Sampler();
        ~Sampler();

        Sampler(Sampler const&) = delete;
        Sampler(Sampler&&) = delete;
        Sampler& operator=(Sampler const&) = delete;
        Sampler& operator=(Sampler&&) = delete;

        VkSampler getHandle() const { return m_handle; }

    private:
        VkSampler m_handle = VK_NULL_HANDLE;
    };
}
