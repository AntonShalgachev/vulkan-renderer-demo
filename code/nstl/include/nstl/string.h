#pragma once

#include "buffer.h"

#include "hash.h"

#include <stddef.h>

namespace nstl
{
    class string_view;

    class string
    {
    public:
        string(char const* str = "");
        string(string_view str);
        string(char const* str, size_t length);

        size_t length() const;
        size_t size() const;
        int slength() const;
        bool empty() const;
        void reserve(size_t capacity);
        char const* c_str() const;
        char* data();
        char const* data() const;
        char& back();
        char const& back() const;

        void resize(size_t size);
        void append(char const* str, size_t length);

        size_t find_last_of(string_view chars) const;
        string substr(size_t offset, size_t length = npos) const;

        char const* begin() const;
        char const* end() const;

        string& operator+=(char rhs);
        string& operator+=(char const* rhs);
        string& operator+=(string const& rhs);
        string& operator+=(string_view const& rhs);

        bool operator==(string const& rhs) const;
        bool operator==(string_view const& rhs) const;
        bool operator==(char const* rhs) const;

        operator string_view() const;

        static size_t npos;

    private:
        void validateIsNullTerminated();

    private:
        Buffer m_buffer;
    };

    string operator+(string lhs, char rhs);
    string operator+(string lhs, char const* rhs);
    string operator+(string lhs, string const& rhs);
    string operator+(string lhs, string_view const& rhs);

    template<>
    struct hash<string>
    {
        size_t operator()(string const& value);
    };
}
