add_library(tiny_ktx
    "include/tiny_ktx/tiny_ktx.h"
    "include/tiny_ktx/stream.h"

    "src/tiny_ktx.cpp"
)

demo_set_common_properties(tiny_ktx)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(tiny_ktx PUBLIC -Wno-unsafe-buffer-usage)
endif()

target_include_directories(tiny_ktx PUBLIC
    "include"
)

target_link_libraries(tiny_ktx
)
