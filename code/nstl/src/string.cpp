#include "nstl/string.h"

#include "nstl/assert.h"
#include "nstl/string_view.h"

#include <string.h>

nstl::String::String(char const* str) : String(str, strlen(str)) {}

nstl::String::String(StringView str) : String(str.data(), str.length()) {}

nstl::String::String(char const* str, size_t length)
{
    resize(length);

    NSTL_ASSERT(str);
    m_buffer.copy(str, length);

    validateIsNullTerminated();
}

size_t nstl::String::length() const
{
    if (m_buffer.size() > 0)
        return m_buffer.size() - 1;
    return 0;
}

int nstl::String::slength() const
{
    return static_cast<int>(length());
}

bool nstl::String::empty() const
{
    return length() == 0;
}

void nstl::String::reserve(size_t capacity)
{
    validateIsNullTerminated();

    size_t requiredBufferSize = capacity + 1;

    if (requiredBufferSize > m_buffer.capacity())
    {
        Buffer buffer{ requiredBufferSize, sizeof(char) };
        buffer.copy(m_buffer.data(), length());
        *buffer.get(length()) = '\0';
        buffer.resize(m_buffer.size());
        m_buffer = nstl::move(buffer);
    }

    validateIsNullTerminated();
}

char const* nstl::String::cStr() const
{
    return m_buffer.data();
}

char* nstl::String::data()
{
    return m_buffer.data();
}

char const* nstl::String::data() const
{
    return m_buffer.data();
}

char& nstl::String::back()
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get(length() - 1);
}

char const& nstl::String::back() const
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get(length() - 1);
}

void nstl::String::resize(size_t newSize)
{
    validateIsNullTerminated();

    reserve(newSize);
    NSTL_ASSERT(m_buffer.capacity() >= newSize + 1);

    m_buffer.resize(newSize + 1);
    *m_buffer.get(newSize) = '\0';

    NSTL_ASSERT(m_buffer.size() == newSize + 1);

    validateIsNullTerminated();
}

void nstl::String::append(char const* str, size_t length)
{
    validateIsNullTerminated();

    size_t oldSize = this->length();
    resize(oldSize + length);
    memcpy(m_buffer.data() + oldSize, str, length);

    validateIsNullTerminated();
}

char const* nstl::String::begin() const
{
    NSTL_ASSERT(m_buffer.size() != 0);
    return m_buffer.data();
}

char const* nstl::String::end() const
{
    return begin() + length();
}

nstl::String& nstl::String::operator+=(char rhs)
{
    append(&rhs, 1);
    return *this;
}

nstl::String& nstl::String::operator+=(char const* rhs)
{
    append(rhs, strlen(rhs));
    return *this;
}

nstl::String& nstl::String::operator+=(String const& rhs)
{
    append(rhs.cStr(), rhs.length());
    return *this;
}

nstl::String& nstl::String::operator+=(StringView const& rhs)
{
    append(rhs.data(), rhs.length());
    return *this;
}

bool nstl::String::operator==(String const& rhs) const
{
    return StringView{ *this } == StringView{ rhs };
}

bool nstl::String::operator==(StringView const& rhs) const
{
    return StringView{ *this } == rhs;
}

bool nstl::String::operator==(char const* rhs) const
{
    return StringView{ *this } == StringView{ rhs };
}

nstl::String::operator nstl::StringView() const
{
    return StringView{ cStr(), length() };
}

void nstl::String::validateIsNullTerminated()
{
    if (m_buffer.size() > 0)
    {
        NSTL_ASSERT(*end() == '\0');
    }
}

nstl::String nstl::operator+(String lhs, char rhs)
{
    return lhs += rhs;
}

nstl::String nstl::operator+(String lhs, char const* rhs)
{
    return lhs += rhs;
}

nstl::String nstl::operator+(String lhs, String const& rhs)
{
    return lhs += rhs;
}

nstl::String nstl::operator+(String lhs, StringView const& rhs)
{
    return lhs += rhs;
}

size_t nstl::Hash<nstl::String>::operator()(String const& value)
{
    return Hash<StringView>{}(value);
}
