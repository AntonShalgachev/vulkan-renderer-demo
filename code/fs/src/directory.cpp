#include "fs/directory.h"

#include "path/walker.h"

#include "platform/filesystem.h"

void fs::create_directory(nstl::string_view path)
{
    return platform::create_directory(path);
}

void fs::create_directories(nstl::string_view path)
{
    for (nstl::string_view part : path::walker{ path })
        create_directory(part);
}
