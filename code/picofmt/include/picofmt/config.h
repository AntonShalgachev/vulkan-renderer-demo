#pragma once

#ifdef PICOFMT_INTERNAL_IMPLEMENTATION
#error Internal implementation should not depend on user-provided types
#endif

#if !defined(PICOFMT_CUSTOM_STRING_VIEW)
#include <string_view>
#define PICOFMT_CUSTOM_STRING_VIEW std::string_view
#endif

#include <assert.h> // TODO remove?

namespace picofmt
{
    using string_view = PICOFMT_CUSTOM_STRING_VIEW;
}

#define PICOFMT_ASSERT(...) assert(__VA_ARGS__)