option(NSTL_ENABLE_ASSERTS "Configure NSTL to enable asserts" ON)

add_library(nstl # non-standard template library
    "pch.h"

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
    "include/nstl/sprintf.h"
    "include/nstl/string_builder.h"
    "include/nstl/allocator.h"
    "include/nstl/malloc_allocator.h"
    "include/nstl/aligned_storage.h"
    "include/nstl/static_vector.h"
    "include/nstl/scope_exit.h"
    "include/nstl/blob.h"
    "include/nstl/tuple.h"
    "include/nstl/source_location.h"
    "include/nstl/blob_view.h"
    "include/nstl/sort.h"
    "include/nstl/event.h"
    "include/nstl/flags_enum.h"
    "include/nstl/alignment.h"

    "src/string.cpp"
    "src/string_view.cpp"
    "src/new.cpp"
    "src/buffer.cpp"
    "src/hash.cpp"
    "src/sprintf.cpp"
    "src/string_builder.cpp"
    "src/allocator.cpp"
    "src/malloc_allocator.cpp"
    "src/blob.cpp"
    "src/blob_view.cpp"
    "src/alignment.cpp"

    "nstl.natvis"
)

target_include_directories(nstl
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

demo_set_common_properties(nstl)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(nstl PUBLIC -Wno-unsafe-buffer-usage) # TODO: revisit
endif()

if(DEMO_PCH)
    target_precompile_headers(nstl PRIVATE "pch.h")
endif()

if (${NSTL_ENABLE_ASSERTS})
    target_compile_definitions(nstl PUBLIC NSTL_CONFIG_ENABLE_ASSERTS=1)
endif()

target_link_libraries(nstl
    memory
)
