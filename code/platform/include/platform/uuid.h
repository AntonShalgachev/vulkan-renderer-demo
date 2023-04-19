#pragma once

#include "nstl/span.h"
#include "nstl/string.h"
#include "nstl/string_view.h"

#include <stdint.h>

namespace platform
{
    bool uuid_generate(nstl::span<uint8_t> bytes);
    bool uuid_from_string(nstl::string_view str, nstl::span<uint8_t> bytes);
    nstl::string uuid_to_string(nstl::span<uint8_t const> bytes);
}
