#pragma once

#include "config.h"

namespace picofmt
{
    // TODO rename to context?

    struct writer
    {
        virtual ~writer() = default;
        virtual bool write(string_view str) = 0;
        virtual void report_error(string_view str) = 0;
    };
}
