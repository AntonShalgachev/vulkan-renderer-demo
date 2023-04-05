#pragma once

#include "nstl/span.h"
#include "nstl/optional.h"
#include "nstl/string_view.h"

#define TINY_CTTI_CUSTOM_STRING_VIEW nstl::string_view
#define TINY_CTTI_CUSTOM_SPAN nstl::span
#define TINY_CTTI_CUSTOM_OPTIONAL nstl::optional

#include "tiny_ctti/tiny_ctti.hpp"
