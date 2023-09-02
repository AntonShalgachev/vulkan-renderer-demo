#pragma once

#include <stddef.h>
#include <string.h>

namespace picofmt::detail
{
    struct simple_string_view
    {
        constexpr simple_string_view() = default;
        simple_string_view(char const* str) : simple_string_view(str, str ? strlen(str) : 0) {}
        constexpr simple_string_view(char const* str, size_t length) : data(str), length(length) {}

        constexpr bool empty() const { return length == 0; }

        simple_string_view substr(size_t offset, size_t len = npos) const
        {
            if (len > length - offset)
                len = length - offset;

            return simple_string_view{ data + offset, len };
        }

        size_t find(char c, size_t pos = 0) const
        {
            for (size_t i = pos; i < length; i++)
            {
                if (data[i] == c)
                    return i;
            }

            return npos;
        }

        constexpr char const& operator[](size_t index) const { return data[index]; }

        inline static size_t npos = static_cast<size_t>(-1);

        char const* data = nullptr;
        size_t length = 0;
    };
}
