#pragma once

#include "nstl/span.h"
#include "nstl/optional.h"
#include "nstl/string_view.h"
#include "nstl/tuple.h"
#include "nstl/sequence.h"

#define TINY_CTTI_CUSTOM_STRING_VIEW nstl::string_view
#define TINY_CTTI_CUSTOM_SPAN nstl::span
#define TINY_CTTI_CUSTOM_OPTIONAL nstl::optional
#define TINY_CTTI_CUSTOM_TUPLE nstl::tuple
#define TINY_CTTI_CUSTOM_INDEX_SEQUENCE nstl::index_sequence
#define TINY_CTTI_CUSTOM_MAKE_INDEX_SEQUENCE nstl::make_index_sequence

#include "tiny_ctti/tiny_ctti.hpp"
