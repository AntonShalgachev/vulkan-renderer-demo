#pragma once

#include "assert.h"
#include "hash.h"

#include <stddef.h>

namespace nstl
{
    class string_view
    {
    public:
        constexpr string_view() = default;
        string_view(char const* str);
        constexpr string_view(char const* str, size_t length) : m_str(str), m_length(length) {}

        size_t length() const;
        size_t size() const;
        int slength() const;
        bool empty() const;
        char const* data() const;

        size_t find(char c, size_t pos = 0) const;
        size_t find(string_view str, size_t pos = 0) const;
        size_t find_first_of(string_view chars) const;
        size_t find_last_of(string_view chars) const;
        string_view substr(size_t offset, size_t length = npos) const;
        bool starts_with(string_view str) const;
        bool ends_with(string_view str) const;

        char const* begin() const;
        char const* end() const;

        char const& operator[](size_t index) const;

        static size_t npos;

        // TODO add more operators
        friend bool operator==(string_view const& lhs, string_view const& rhs);
        friend bool operator!=(string_view const& lhs, string_view const& rhs);
        friend bool operator<(string_view const& lhs, string_view const& rhs);

    private:
        char const* m_str = nullptr;
        size_t m_length = 0;
    };

    template<>
    struct hash<string_view>
    {
        size_t operator()(string_view const& value);
    };
}
