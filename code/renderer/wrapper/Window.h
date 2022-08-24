#pragma once

#include <vector>

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
        virtual std::vector<char const*> const& getRequiredInstanceExtensions() const = 0;
    };
}
