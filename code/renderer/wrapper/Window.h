#pragma once

#include "nstl/vector.h"

#include <functional>

namespace vko
{
    class Surface;
    class Instance;

    class Window
    {
    public:
        virtual Surface createSurface(vko::Instance const& instance) const = 0;
        virtual std::size_t getWidth() const = 0;
        virtual std::size_t getHeight() const = 0;
        virtual nstl::vector<char const*> const& getRequiredInstanceExtensions() const = 0;

        virtual void waitUntilInForeground() const = 0;
        virtual void addResizeCallback(std::function<void(int, int)> callback) = 0;
    };
}
