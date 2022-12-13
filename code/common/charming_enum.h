#pragma once

#include "nstl/span.h"
#include "nstl/optional.h"
#include "nstl/string_view.h"

#define CHARMING_ENUM_CUSTOM_STRING_VIEW nstl::string_view
#define CHARMING_ENUM_CUSTOM_SPAN nstl::span<T>
#define CHARMING_ENUM_CUSTOM_OPTIONAL nstl::optional<T>
#include "charming_enum/charming_enum.hpp"
