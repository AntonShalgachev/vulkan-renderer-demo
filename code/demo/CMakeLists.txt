add_executable(demo WIN32
    "pch.h"

    "main.cpp"
    
    "DemoApplication.h"
    "DemoApplication.cpp"
    "DemoSceneDrawer.h"
    "DemoSceneDrawer.cpp"

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

target_include_directories(demo PRIVATE ".")

demo_set_common_properties(demo)

if(DEMO_PCH)
    target_precompile_headers(demo PRIVATE "pch.h")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #
elseif(MSVC)
    target_compile_options(demo PRIVATE /wd4324) # structure was padded due to alignment specifier
endif()

set_property(TARGET demo PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_mark_includes_system(demo imgui dds-ktx::dds-ktx cgltf)

target_link_libraries(demo
    imgui
    common
    stb
    coil::coil
    logging
    yyjsoncpp
    dds-ktx::dds-ktx
    cgltf
    memory
    editor
    mt
    gfx
    gfx_vk
    gfx_vk_win64
)
