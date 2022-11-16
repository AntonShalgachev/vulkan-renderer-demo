#pragma once

#include "hash.h"

#include <stddef.h>

namespace nstl
{
    class string_view
    {
    public:
        string_view(char const* str = "");
        string_view(char const* str, size_t length);

        size_t length() const;
        size_t size() const;
        int slength() const;
        bool empty() const;
        char const* data() const;

        string_view substr(size_t offset, size_t length) const;

        char const* begin() const;
        char const* end() const;

        char const& operator[](size_t index) const;

    private:
        char const* m_str = nullptr;
        size_t m_length = 0;
    };

    bool operator==(string_view const& lhs, string_view const& rhs);
    bool operator!=(string_view const& lhs, string_view const& rhs);

    template<>
    struct Hash<string_view>
    {
        size_t operator()(string_view const& value);
    };
}
