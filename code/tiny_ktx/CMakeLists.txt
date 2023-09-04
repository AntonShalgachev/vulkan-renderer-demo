add_library(tiny_ktx
    "include/tiny_ktx/tiny_ktx.h"
    "include/tiny_ktx/stream.h"

    "src/tiny_ktx.cpp"
)

target_compile_features(tiny_ktx PUBLIC cxx_std_20)

target_include_directories(tiny_ktx PUBLIC
    "include"
)

target_link_libraries(tiny_ktx
)
