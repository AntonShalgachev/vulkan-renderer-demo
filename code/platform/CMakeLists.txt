add_library(platform
    "include/platform/memory.h"
    "include/platform/filesystem.h"
    "include/platform/uuid.h"
    "include/platform/threading.h"
    "src/windows/windows_common.h"
    "include/platform/debug.h"
    "include/platform/startup.h"
    "include/platform/time.h"

    "src/windows/memory.cpp"
    "src/windows/filesystem.cpp"
    "src/windows/uuid.cpp"
    "src/windows/threading.cpp"
    "src/windows/windows_common.cpp"
    "src/windows/debug.cpp"
    "src/windows/startup.cpp"
    "src/windows/time.cpp"
)

demo_set_common_properties(platform)

target_include_directories(platform PUBLIC
    "include"
)

target_link_libraries(platform
    nstl
    "Rpcrt4.lib"
    fs
)
