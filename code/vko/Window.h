#pragma once

#include "nstl/vector.h"
#include "nstl/function.h"

namespace vko
{
    class Surface;
    class Instance;

    class Window
    {
    public:
        virtual Surface createSurface(vko::Instance const& instance) const = 0;
        virtual std::size_t getFramebufferWidth() const = 0;
        virtual std::size_t getFramebufferHeight() const = 0;
        virtual nstl::vector<char const*> const& getRequiredInstanceExtensions() const = 0;

        virtual void waitUntilInForeground() const = 0;
        virtual void addFramebufferResizeCallback(nstl::function<void(int, int)> callback) = 0;
    };
}
