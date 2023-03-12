#pragma once

#include "path/path.h"

#include "nstl/string_view.h"

namespace path
{
    class walker_iterator
    {
    public:
        walker_iterator() = default;
        walker_iterator(nstl::string_view path, size_t offset = 0);

        nstl::string_view operator*() { return m_part; }

        bool operator==(walker_iterator const&) const = default;

        walker_iterator& operator++();

    private:
        nstl::string_view m_path;
        size_t m_next_separator_pos = 0;

        nstl::string_view m_part;
    };

    class walker
    {
    public:
        walker(nstl::string_view path);

        walker_iterator begin();
        walker_iterator end();

    private:
        nstl::string_view m_path;
    };
}
