#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <stdint.h>

#include "nstl/string.h"

namespace platform_win64
{
    struct error
    {
        DWORD code;
        nstl::string message;
    };

    // TODO move somewhere
    class wstring
    {
    public:
        wstring(nstl::any_allocator alloc = {}) : wstring(0, nstl::move(alloc)) {}
        wstring(size_t length, nstl::any_allocator alloc = {})
            : m_buffer(length, sizeof(wchar_t), alignof(wchar_t), nstl::move(alloc))
        {
            m_buffer.resize(length);
        }

        size_t length() const { return m_buffer.size(); }
        wchar_t const* c_str() const { return data(); }
        wchar_t* data() { return m_buffer.get<wchar_t>(0); }
        wchar_t const* data() const { return m_buffer.get<wchar_t>(0); }

        wchar_t& operator[](size_t index) { assert(index < m_buffer.size()); return *m_buffer.get<wchar_t>(index); }

    private:
        nstl::buffer m_buffer;
    };

    error get_last_error();

    wstring convert_to_wstring(nstl::string_view str);
    nstl::string convert_to_string(WCHAR const* str);
}
