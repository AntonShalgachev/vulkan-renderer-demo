add_library(platform INTERFACE
    "include/platform/memory.h"
    "include/platform/filesystem.h"
    "include/platform/uuid.h"
    "include/platform/threading.h"
    "include/platform/debug.h"
    "include/platform/startup.h"
    "include/platform/time.h"
    "include/platform/window.h"
)

target_include_directories(platform INTERFACE
    "include"
)

target_link_libraries(platform INTERFACE
    nstl
    fs
)
