add_library(platform_msvc INTERFACE
    "include/platform/atomic.h"
)

target_include_directories(platform_msvc INTERFACE
    "include"
)
