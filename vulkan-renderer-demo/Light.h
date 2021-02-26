#pragma once
#include "Transform.h"

namespace vkr
{
    class Light
    {
    public:
        Transform& getTransform() { return m_transform; }
        Transform const& getTransform() const { return m_transform; }

    private:
        Transform m_transform;
    };
}
