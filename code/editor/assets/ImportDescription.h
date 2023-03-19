#pragma once

#include "nstl/span.h"
#include "nstl/string_view.h"

namespace editor::assets
{
    struct ImportDescription
    {
        nstl::span<unsigned char const> content;
        nstl::string_view parentDirectory;
        nstl::string_view name;
        nstl::string_view extension;
    };
}
