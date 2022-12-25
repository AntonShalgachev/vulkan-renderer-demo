add_library(vkr
    "pch.h"

    "Application.cpp"
    "Application.h"
)

target_include_directories(vkr PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(vkr PUBLIC cxx_std_20)

if(DEMO_PCH)
    target_precompile_headers(vkr PRIVATE "pch.h")
endif()

target_link_libraries(vkr
    vko
)
