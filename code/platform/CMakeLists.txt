add_library(platform
    "include/platform/memory.h"
    "include/platform/filesystem.h"
    "include/platform/uuid.h"
    "include/platform/threading.h"
    "src/windows/windows_common.h"

    "src/windows/memory.cpp"
    "src/windows/filesystem.cpp"
    "src/windows/uuid.cpp"
    "src/windows/threading.cpp"
    "src/windows/windows_common.cpp"
)

target_compile_features(platform PUBLIC cxx_std_20)

target_include_directories(platform PUBLIC
    "include"
)

target_link_libraries(platform
    nstl
    "Rpcrt4.lib"
    fs
)
