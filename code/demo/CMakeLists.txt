set(ALL_FILES
    "pch.h"

    "main.cpp"
    
    "DemoApplication.h"
    "DemoApplication.cpp"

    "ImGuiDrawer.h"
    "ImGuiDrawer.cpp"
    "ImGuiPlatform.h"
    "ImGuiPlatform.cpp"

    "ScopedDebugCommands.h"
    "ScopedDebugCommands.cpp"
    "CommandProxy.h"
    "CommandMetadata.h"
    "CommandMetadata.cpp"
    "ShaderPackage.h"
    "ShaderPackage.cpp"
    
    "GlfwWindow.cpp"
    "GlfwWindow.h"

    "console/GlmSerializer.h"
    "console/GlmSerializer.cpp"
    "console/EnumSerializer.h"
    "console/EditorSerializer.h"
    "console/EditorSerializer.cpp"
    
    "services/DebugConsoleService.h"
    "services/DebugConsoleService.cpp"
    "services/CommandLineService.h"
    "services/CommandLineService.cpp"
    "services/DebugDrawService.h"
    "services/DebugDrawService.cpp"
    "services/Services.h"
    "services/Services.cpp"
    "services/ServiceContainer.h"

    "ui/NotificationManager.h"
    "ui/NotificationManager.cpp"
    "ui/DebugConsoleWidget.h"
    "ui/DebugConsoleWidget.cpp"
    "ui/MemoryViewerWindow.h"
    "ui/MemoryViewerWindow.cpp"
)

add_executable(demo WIN32 ${ALL_FILES})

target_include_directories(demo PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

demo_set_common_properties(demo)

if(DEMO_PCH)
    target_precompile_headers(demo PRIVATE "pch.h")
endif()

set_property(TARGET demo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(demo
    vko # TODO remove
    vkgfx # TODO remove
    glfw
    imgui
    common
    stb
    coil::coil
    logging
    yyjsoncpp
    dds-ktx::dds-ktx
    cgltf
    memory
    editor::assets
    mt
    gfx
    gfx_vk
)
