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
    tiny_ctti
)

if (WIN32)
    target_link_libraries(platform INTERFACE platform_win64)
else()
    message(FATAL_ERROR "Unknown platform")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(platform INTERFACE platform_clang)
elseif(MSVC)
    target_link_libraries(platform INTERFACE platform_msvc)
else()
    message(FATAL_ERROR "Unknown compiler")
endif()
