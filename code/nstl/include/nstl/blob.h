#pragma once

#include "buffer.h"
#include "allocator.h"

#include "nstl/blob_view.h"

#include <stddef.h>

namespace nstl
{
    class blob
    {
    public:
        blob(size_t size = 0, any_allocator alloc = {});

        size_t size() const;
        void* data();
        void const* data() const;
        char* cdata();
        char const* cdata() const;
        unsigned char* ucdata();
        unsigned char const* ucdata() const;

        operator blob_view() const;

    private:
        buffer m_buffer;
    };
}
