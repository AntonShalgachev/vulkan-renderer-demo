add_library(platform_clang INTERFACE
    "include/platform/atomic.h"
)

target_include_directories(platform_clang INTERFACE
    "include"
)
