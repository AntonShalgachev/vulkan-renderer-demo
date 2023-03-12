#pragma once

#include "nstl/string_view.h"

namespace fs
{
    void create_directory(nstl::string_view path);
    void create_directories(nstl::string_view path);
}
