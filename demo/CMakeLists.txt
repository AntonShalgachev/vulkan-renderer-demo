set(PROJECT_NAME demo)

set(ALL_FILES
    "vulkan-renderer-demo.cpp"

    "ui/NotificationManager.h"
    "ui/NotificationManager.cpp"
    "ui/DebugConsole.h"
    "ui/DebugConsole.cpp"
)

add_executable(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME}
    librenderer
    libimguibackend
    imgui
    glm::glm
    tinygltf
    magic_enum
)
