#include "nstl/string.h"

#include "nstl/assert.h"
#include "nstl/string_view.h"

#include <string.h>

size_t nstl::string::npos = static_cast<size_t>(-1);

nstl::string::string(char const* str) : string(str, strlen(str)) {}

nstl::string::string(string_view str) : string(str.data(), str.length()) {}

nstl::string::string(char const* str, size_t length)
{
    resize(length);

    NSTL_ASSERT(str);
    m_buffer.copy(str, length);

    validateIsNullTerminated();
}

size_t nstl::string::length() const
{
    if (m_buffer.size() > 0)
        return m_buffer.size() - 1;
    return 0;
}

size_t nstl::string::size() const
{
    if (m_buffer.size() > 0)
        return m_buffer.size() - 1;
    return 0;
}

size_t nstl::string::capacity() const
{
    if (m_buffer.capacity() > 0)
        return m_buffer.capacity() - 1;
    return 0;
}

int nstl::string::slength() const
{
    return static_cast<int>(length());
}

bool nstl::string::empty() const
{
    return length() == 0;
}

void nstl::string::reserve(size_t capacity)
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

char const* nstl::string::c_str() const
{
    return m_buffer.data();
}

char* nstl::string::data()
{
    return m_buffer.data();
}

char const* nstl::string::data() const
{
    return m_buffer.data();
}

char& nstl::string::back()
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get(length() - 1);
}

char const& nstl::string::back() const
{
    NSTL_ASSERT(!empty());
    return *m_buffer.get(length() - 1);
}

void nstl::string::resize(size_t newSize)
{
    validateIsNullTerminated();

    reserve(newSize);
    NSTL_ASSERT(m_buffer.capacity() >= newSize + 1);

    m_buffer.resize(newSize + 1);
    *m_buffer.get(newSize) = '\0';

    NSTL_ASSERT(m_buffer.size() == newSize + 1);

    validateIsNullTerminated();
}

void nstl::string::append(char const* str, size_t length)
{
    validateIsNullTerminated();

    size_t oldSize = this->length();
    resize(oldSize + length);
    memcpy(m_buffer.data() + oldSize, str, length);

    validateIsNullTerminated();
}

size_t nstl::string::find_last_of(string_view chars) const
{
    return static_cast<string_view>(*this).find_last_of(chars);
}

nstl::string nstl::string::substr(size_t offset, size_t length) const
{
    return string{ static_cast<string_view>(*this).substr(offset, length) };
}

char const* nstl::string::begin() const
{
    NSTL_ASSERT(m_buffer.size() != 0);
    return m_buffer.data();
}

char const* nstl::string::end() const
{
    return begin() + length();
}

nstl::string& nstl::string::operator+=(char rhs)
{
    append(&rhs, 1);
    return *this;
}

nstl::string& nstl::string::operator+=(char const* rhs)
{
    append(rhs, strlen(rhs));
    return *this;
}

nstl::string& nstl::string::operator+=(string const& rhs)
{
    append(rhs.c_str(), rhs.length());
    return *this;
}

nstl::string& nstl::string::operator+=(string_view const& rhs)
{
    append(rhs.data(), rhs.length());
    return *this;
}

bool nstl::string::operator==(string const& rhs) const
{
    return string_view{ *this } == string_view{ rhs };
}

bool nstl::string::operator==(string_view const& rhs) const
{
    return string_view{ *this } == rhs;
}

bool nstl::string::operator==(char const* rhs) const
{
    return string_view{ *this } == string_view{ rhs };
}

nstl::string::operator nstl::string_view() const
{
    return string_view{ c_str(), length() };
}

void nstl::string::validateIsNullTerminated()
{
    if (m_buffer.size() > 0)
    {
        NSTL_ASSERT(*end() == '\0');
    }
}

nstl::string nstl::operator+(string lhs, char rhs)
{
    return lhs += rhs;
}

nstl::string nstl::operator+(string lhs, char const* rhs)
{
    return lhs += rhs;
}

nstl::string nstl::operator+(string lhs, string const& rhs)
{
    return lhs += rhs;
}

nstl::string nstl::operator+(string lhs, string_view const& rhs)
{
    return lhs += rhs;
}

size_t nstl::hash<nstl::string>::operator()(string const& value)
{
    return hash<string_view>{}(value);
}
