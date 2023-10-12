#pragma once

#include "nstl/span.h"
#include "nstl/string.h"
#include "nstl/string_view.h"

namespace nstl
{
    // view of typeless data

    class blob_view
    {
    public:
        blob_view() = default;
        blob_view(void const* data, size_t size) : m_data(data), m_size(size) {}

        blob_view(string_view str) : blob_view(str.data(), str.size()) {}
        explicit blob_view(string const& str) : blob_view(str.data(), str.size()) {} // TODO feels like a hack to allow string -> blob_view conversion. Remove?
        blob_view(span<unsigned char> data) : blob_view(data.data(), data.size()) {}
        blob_view(span<unsigned char const> data) : blob_view(data.data(), data.size()) {}

        size_t size() const { return m_size; }
        bool empty() const { return m_size == 0; }
        void const* data() const { return m_data; }
        char const* cdata() const { return static_cast<char const*>(m_data); }
        unsigned char const* ucdata() const { return static_cast<unsigned char const*>(m_data); }

        blob_view subview(size_t offset, size_t count = static_cast<size_t>(-1)) const;

    private:
        void const* m_data = nullptr;
        size_t m_size = 0;
    };
}
