#pragma once

namespace yyjsoncpp
{
    enum class write_flags
    {
        pretty = 1 << 0,
        escape_unicode = 1 << 1,
        escsape_slashes = 1 << 2,
        allow_inf_and_nan = 1 << 3,
        inf_and_nan_as_null = 1 << 4,
        allow_invalid_unicode = 1 << 5,
    };

    inline write_flags operator|(write_flags lhs, write_flags rhs)
    {
        return static_cast<write_flags>(static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs));
    }

    inline write_flags operator&(write_flags lhs, write_flags rhs)
    {
        return static_cast<write_flags>(static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs));
    }
}
