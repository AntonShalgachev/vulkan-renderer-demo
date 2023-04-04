#pragma once

#include "buffer.h"
#include "allocator.h"

#include <stddef.h>

namespace nstl
{
    class string_view;
    template<typename T> class span;

    class blob
    {
    public:
        blob(size_t size, any_allocator alloc = {});

        size_t size() const;
        void* data();
        void const* data() const;
        char* cdata();
        char const* cdata() const;
        unsigned char* ucdata();
        unsigned char const* ucdata() const;

        operator string_view() const;
        operator span<unsigned char>();
        operator span<unsigned char const>() const;

    private:
        buffer m_buffer;
    };
}
