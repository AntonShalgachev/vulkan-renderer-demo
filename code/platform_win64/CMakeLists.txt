add_library(platform_win64
    "include/platform_win64/window.h"

    "src/memory.cpp"
    "src/filesystem.cpp"
    "src/uuid.cpp"
    "src/threading.cpp"
    "src/common.cpp"
    "src/common.h"
    "src/debug.cpp"
    "src/startup.cpp"
    "src/time.cpp"
    "src/window.cpp"
)

demo_set_common_properties(platform_win64)

target_include_directories(platform_win64 PUBLIC
    "include"
)

target_link_libraries(platform_win64
    platform
    "Rpcrt4.lib"
    glfw # TODO remove
)
