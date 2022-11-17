#pragma once

#include "hash.h"

#include <stddef.h>

#include <compare> // TODO avoid including this header?

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

        size_t find(char c, size_t pos = 0) const;
        size_t find(string_view str, size_t pos = 0) const;
        string_view substr(size_t offset, size_t length = static_cast<size_t>(-1)) const;
        bool starts_with(string_view str) const;

        char const* begin() const;
        char const* end() const;

        char const& operator[](size_t index) const;

        std::strong_ordering operator<=>(string_view const& rhs) const;

        static size_t npos;

    private:
        char const* m_str = nullptr;
        size_t m_length = 0;
    };

    bool operator==(string_view const& lhs, string_view const& rhs);
    bool operator!=(string_view const& lhs, string_view const& rhs);

    template<>
    struct hash<string_view>
    {
        size_t operator()(string_view const& value);
    };
}
