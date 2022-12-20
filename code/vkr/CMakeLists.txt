add_library(vkr
    "pch.h"

    "Application.cpp"
    "Application.h"
    "PhysicalDeviceSurfaceContainer.cpp"
    "PhysicalDeviceSurfaceContainer.h"
    "PhysicalDeviceSurfaceParameters.cpp"
    "PhysicalDeviceSurfaceParameters.h"
    "QueueFamilyIndices.cpp"
    "QueueFamilyIndices.h"
)

target_include_directories(vkr PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(vkr PUBLIC cxx_std_20)

target_precompile_headers(vkr PRIVATE "pch.h")

target_link_libraries(vkr
    vko
)
