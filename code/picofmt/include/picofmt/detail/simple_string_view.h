#pragma once

#include <stddef.h>
#include <string.h>

namespace picofmt::detail
{
    class simple_string_view
    {
    public:
        constexpr simple_string_view() = default;
        simple_string_view(char const* str) : simple_string_view(str, strlen(str)) {}
        constexpr simple_string_view(char const* str, size_t length) : m_str(str), m_length(length) {}

        constexpr char const* data() const { return m_str; }
        constexpr size_t length() const { return m_length; }
        constexpr size_t size() const { return m_length; }
        constexpr bool empty() const { return m_length == 0; }

        simple_string_view substr(size_t offset, size_t length = npos) const
        {
            if (length > m_length - offset)
                length = m_length - offset;

            return simple_string_view{ m_str + offset, length };
        }

        size_t find(char c, size_t pos = 0) const
        {
            for (size_t i = pos; i < m_length; i++)
            {
                if (m_str[i] == c)
                    return i;
            }

            return npos;
        }

        constexpr char const& operator[](size_t index) const { return m_str[index]; }

        friend bool operator==(simple_string_view const& lhs, simple_string_view const& rhs)
        {
            if (lhs.m_length != rhs.m_length)
                return false;

            return strncmp(lhs.m_str, rhs.m_str, lhs.m_length) == 0;
        }

    public:
        inline static size_t npos = static_cast<size_t>(-1);

    private:
        char const* m_str = nullptr;
        size_t m_length = 0;
    };
}
