#include "path/walker.h"

path::walker_iterator::walker_iterator(nstl::string_view path, size_t offset /*= 0*/)
{
    if (offset >= path.length())
        return;

    m_path = path;

    m_next_separator_pos = path.find(path::separator, offset);
    if (m_next_separator_pos == nstl::string_view::npos)
        m_next_separator_pos = path.size();

    m_part = path.substr(0, m_next_separator_pos);
}

path::walker_iterator& path::walker_iterator::operator++()
{
    *this = walker_iterator{ m_path, m_next_separator_pos + 1 };
    return *this;
}

path::walker::walker(nstl::string_view path) : m_path(path)
{

}

path::walker_iterator path::walker::begin()
{
    return walker_iterator{ m_path };
}

path::walker_iterator path::walker::end()
{
    return walker_iterator{};
}
