#include "nstl/StringView.h"

#include "nstl/Assert.h"

#include <string.h>

nstl::StringView::StringView(char const* str) : StringView(str, strlen(str)) {}

nstl::StringView::StringView(char const* str, size_t length) : m_str(str), m_length(length)
{
    NSTL_ASSERT(m_str);
}

size_t nstl::StringView::length() const
{
    return m_length;
}

int nstl::StringView::slength() const
{
    return static_cast<int>(length());
}

bool nstl::StringView::empty() const
{
    return length() == 0;
}

char const* nstl::StringView::data() const
{
    NSTL_ASSERT(m_str);
    return m_str;
}

nstl::StringView nstl::StringView::substr(size_t offset, size_t length) const
{
    NSTL_ASSERT(offset <= m_length);
    NSTL_ASSERT(length <= m_length);
    NSTL_ASSERT(offset + length <= m_length);
    return StringView{ m_str + offset, length };
}

char const* nstl::StringView::begin() const
{
    NSTL_ASSERT(m_str);
    return m_str;
}

char const* nstl::StringView::end() const
{
    NSTL_ASSERT(m_str);
    return m_str + m_length;
}

char const& nstl::StringView::operator[](size_t index) const
{
    NSTL_ASSERT(index < m_length);
    return m_str[index];
}

bool nstl::operator==(StringView const& lhs, StringView const& rhs)
{
    if (lhs.length() != rhs.length())
        return false;

    size_t size = lhs.length();

    return memcmp(lhs.data(), rhs.data(), size) == 0;
}

bool nstl::operator!=(StringView const& lhs, StringView const& rhs)
{
    return !(lhs == rhs);
}

size_t nstl::Hash<nstl::StringView>::operator()(StringView const& value)
{
    // djb2 from http://www.cse.yorku.ca/~oz/hash.html

    size_t hash = 5381;

    for (char c : value)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
