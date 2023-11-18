#pragma once

#include "buffer.h"

#include "hash.h"
#include "allocator.h"

#include <stddef.h>

namespace nstl
{
    class string_view;

    class string
    {
    public:
        string(any_allocator alloc = {});
        string(size_t length, any_allocator alloc = {});
        string(char const* str, any_allocator alloc = {});
        string(string_view str, any_allocator alloc = {});
        string(char const* str, size_t length, any_allocator alloc = {});

        size_t length() const;
        size_t size() const;
        size_t capacity() const;
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
        void push_back(char c);

        size_t find_last_of(string_view chars) const;
        string substr(size_t offset, size_t length = npos) const;

        char* begin();
        char const* begin() const;
        char* end();
        char const* end() const;

        char& operator[](size_t index);
        char const& operator[](size_t index) const;

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
        buffer m_buffer;
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
