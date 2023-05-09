#include "nstl/string_view.h"

#include "nstl/assert.h"

#include <string.h>

size_t nstl::string_view::npos = static_cast<size_t>(-1);

nstl::string_view::string_view(char const* str) : string_view(str, str ? strlen(str) : 0) {}

size_t nstl::string_view::length() const
{
    return m_length;
}

size_t nstl::string_view::size() const
{
    return m_length;
}

int nstl::string_view::slength() const
{
    return static_cast<int>(length());
}

bool nstl::string_view::empty() const
{
    return length() == 0;
}

char const* nstl::string_view::data() const
{
    return m_str;
}

size_t nstl::string_view::find(char c, size_t pos) const
{
    for (size_t i = pos; i < m_length; i++)
    {
        if (m_str[i] == c)
            return i;
    }

    return npos;
}

size_t nstl::string_view::find(string_view str, size_t pos) const
{
    // TODO move somewhere

    auto begin1 = m_str + pos;
//     auto end1 = m_str + m_length;
    auto length1 = m_length - pos;
    auto begin2 = str.m_str;
    auto end2 = str.m_str + str.m_length;
    auto length2 = str.m_length;

    size_t index = 0;
    for (auto len = length1; len >= length2; ++index, --len)
    {
        auto temp1 = begin1 + index;

        for (auto temp2 = begin2; ; ++temp1, ++temp2)
        {
            if (temp2 == end2)
                return pos + index;
            if ((*temp1 != *temp2))
                break;
        }
    }

    return string_view::npos;
}

size_t nstl::string_view::find_first_of(string_view chars) const
{
    for (size_t i = 0; i < m_length; i++)
    {
        char c = m_str[i];
        if (chars.find(c) != string_view::npos)
            return i;
    }

    return npos;
}

size_t nstl::string_view::find_last_of(string_view chars) const
{
    for (size_t i = 0; i < m_length; i++)
    {
        char c = m_str[m_length - 1 - i];
        if (chars.find(c) != string_view::npos)
            return m_length - 1 - i;
    }

    return npos;
}

nstl::string_view nstl::string_view::substr(size_t offset, size_t length) const
{
    NSTL_ASSERT(offset <= m_length);

    if (length > m_length - offset)
        length = m_length - offset;

    NSTL_ASSERT(length <= m_length);
    NSTL_ASSERT(offset + length <= m_length);
    return string_view{ m_str + offset, length };
}

bool nstl::string_view::starts_with(string_view str) const
{
    return substr(0, str.length()) == str;
}

bool nstl::string_view::ends_with(string_view str) const
{
    if (str.length() > m_length)
        return false;

    size_t offset = m_length - str.length();
    return substr(offset, str.length()) == str;
}

char const* nstl::string_view::begin() const
{
    if (m_length > 0)
        NSTL_ASSERT(m_str);

    return m_str;
}

char const* nstl::string_view::end() const
{
    if (m_length > 0)
        NSTL_ASSERT(m_str);

    return m_str + m_length;
}

char const& nstl::string_view::operator[](size_t index) const
{
    NSTL_ASSERT(index < m_length);
    return m_str[index];
}

bool nstl::operator==(string_view const& lhs, string_view const& rhs)
{
    if (lhs.empty() && rhs.empty())
        return true;

    if (lhs.m_length != rhs.m_length)
        return false;

    size_t size = lhs.m_length;

    return memcmp(lhs.m_str, rhs.m_str, size) == 0;
}

bool nstl::operator!=(string_view const& lhs, string_view const& rhs)
{
    return !(lhs == rhs);
}

bool nstl::operator<(string_view const& lhs, string_view const& rhs)
{
    size_t common_length = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
    int cmp = strncmp(lhs.data(), rhs.data(), common_length);
    if (cmp != 0)
        return cmp < 0;

    return lhs.size() < rhs.size();
}

size_t nstl::hash<nstl::string_view>::operator()(string_view const& value)
{
    return hash_string(value.data(), value.length());
}
