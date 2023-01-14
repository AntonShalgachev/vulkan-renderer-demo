add_library(platform
    "include/platform/memory.h"
    "src/windows/memory.cpp"
)

target_compile_features(platform PUBLIC cxx_std_20)

target_include_directories(platform PUBLIC
    "include"
)
