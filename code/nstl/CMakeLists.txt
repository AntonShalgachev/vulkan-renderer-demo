set(NSTL_ENABLE_ASSERTS OFF CACHE BOOL "Configure NSTL to enable asserts")

add_library(nstl # custom template library
    "include/nstl/algorithm.h"
    "include/nstl/assert.h"
    "include/nstl/buffer.h"
    "include/nstl/config.h"
    "include/nstl/expected.h"
    "include/nstl/hash.h"
    "include/nstl/new.h"
    "include/nstl/optional.h"
    "include/nstl/sequence.h"
    "include/nstl/string.h"
    "include/nstl/string_view.h"
    "include/nstl/type_traits.h"
    "include/nstl/unexpected.h"
    "include/nstl/unique_ptr.h"
    "include/nstl/unordered_map.h"
    "include/nstl/utility.h"
    "include/nstl/vector.h"
    "include/nstl/span.h"
    "include/nstl/array.h"
    "include/nstl/lexicographical_compare.h"
    "include/nstl/vector_hash.h"
    "include/nstl/function.h"

    "src/string.cpp"
    "src/string_view.cpp"
    "src/new.cpp"
    "src/buffer.cpp"
    "src/hash.cpp"

    "nstl.natvis"
)

target_include_directories(nstl
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(nstl PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(nstl PRIVATE /W4 /WX)
else()
  target_compile_options(nstl PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if (${NSTL_ENABLE_ASSERTS})
    target_compile_definitions(nstl PUBLIC NSTL_CONFIG_ENABLE_ASSERTS=1)
endif()
