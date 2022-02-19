set(PROJECT_NAME libimgui)

set(ALL_FILES
    "imconfig.h"
    "imgui.cpp"
    "imgui.h"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_impl_glfw.cpp"
    "imgui_impl_glfw.h"
    "imgui_impl_vulkan.cpp"
    "imgui_impl_vulkan.h"
    "imgui_internal.h"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "imstb_rectpack.h"
    "imstb_textedit.h"
    "imstb_truetype.h"
)

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME}
    glfw
    Vulkan::Vulkan
)
