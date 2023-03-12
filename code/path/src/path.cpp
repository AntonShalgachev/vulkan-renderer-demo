#include "path/path.h"

// TODO implement properly

namespace
{
    nstl::string_view separators = "/\\";

    nstl::string_view get_last_segment(nstl::string_view path)
    {
        size_t pos = path.find_last_of(separators);
        if (pos == nstl::string_view::npos) // e.g. "file.txt", "folder"
            return path;

        return path.substr(pos + 1);
    }

    nstl::string_view strip_last_segment(nstl::string_view path)
    {
        size_t pos = path.find_last_of(separators);
        if (pos == nstl::string_view::npos) // e.g. "file.txt", "folder"
            return {};

        return path.substr(0, pos);
    }
}

path::parts path::split_into_parts(nstl::string_view path)
{
    nstl::string_view last_segment = get_last_segment(path);

    size_t dot_pos = last_segment.find('.');
    if (dot_pos == nstl::string_view::npos)
        dot_pos = last_segment.size();

    parts res;
    res.full_name = last_segment;
    res.name_without_extension = last_segment.substr(0, dot_pos);
    res.extension = last_segment.substr(dot_pos + 1);
    res.parent_path = strip_last_segment(path);

    return res;
}

nstl::string path::join_array(nstl::span<nstl::string_view const> parts)
{
    if (parts.empty())
        return {};

    size_t length = parts[0].length();
    for (nstl::string_view part : parts.subspan(1))
        length += 1 + part.length(); // separator + part

    nstl::string result;
    result.reserve(length);

    result += parts[0];
    for (nstl::string_view part : parts.subspan(1))
    {
        result += separator;
        result += part;
    }

    assert(result.length() == length);

    return result;
}
