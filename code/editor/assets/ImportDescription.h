#pragma once

#include "nstl/blob_view.h"
#include "nstl/string_view.h"

namespace editor::assets
{
    struct ImportDescription
    {
        nstl::blob_view content;
        nstl::string_view parentDirectory;
        nstl::string_view name;
        nstl::string_view extension; // TODO change to "typeId"?
    };
}
