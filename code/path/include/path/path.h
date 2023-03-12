#pragma once

#include "nstl/string_view.h"
#include "nstl/string.h"
#include "nstl/span.h"

namespace path
{
    inline static const char separator = '/';

    struct parts
    {
        // some/path/to/file.txt
        nstl::string_view parent_path; // some/path/to
        nstl::string_view full_name; // file.txt
        nstl::string_view name_without_extension; // file
        nstl::string_view extension; // txt
    };

    parts split_into_parts(nstl::string_view path);

    nstl::string join_array(nstl::span<nstl::string_view const> parts);

    template<typename... T>
    nstl::string join(T&&... args)
    {
        nstl::string_view parts[sizeof...(args)] = { args... };
        return join_array(parts);
    }

    // TODO implement
    //nstl::string normalize(nstl::string_view path);
}
