#include "nstl/string_view.h"

#include "nstl/assert.h"

#include <string.h>

nstl::string_view::string_view(char const* str) : string_view(str, strlen(str)) {}

nstl::string_view::string_view(char const* str, size_t length) : m_str(str), m_length(length)
{
    NSTL_ASSERT(m_str);
}

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
    NSTL_ASSERT(m_str);
    return m_str;
}

nstl::string_view nstl::string_view::substr(size_t offset, size_t length) const
{
    NSTL_ASSERT(offset <= m_length);
    NSTL_ASSERT(length <= m_length);
    NSTL_ASSERT(offset + length <= m_length);
    return string_view{ m_str + offset, length };
}

char const* nstl::string_view::begin() const
{
    NSTL_ASSERT(m_str);
    return m_str;
}

char const* nstl::string_view::end() const
{
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
    if (lhs.length() != rhs.length())
        return false;

    size_t size = lhs.length();

    return memcmp(lhs.data(), rhs.data(), size) == 0;
}

bool nstl::operator!=(string_view const& lhs, string_view const& rhs)
{
    return !(lhs == rhs);
}

size_t nstl::hash<nstl::string_view>::operator()(string_view const& value)
{
    // djb2 from http://www.cse.yorku.ca/~oz/hash.html

    size_t hash = 5381;

    for (char c : value)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
