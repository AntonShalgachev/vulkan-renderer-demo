#pragma once

#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/optional.h"

namespace yyjsoncpp
{
    using string_view = nstl::string_view;
    using string = nstl::string;
    template<typename T> using optional = nstl::optional<T>;
}
