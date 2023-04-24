cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(vulkan-renderer-demo LANGUAGES C CXX VERSION 1.0)

set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
    CACHE STRING "" FORCE
)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()

option(DEMO_CLANG_TRACE "Generate Clang trace files" OFF)
option(DEMO_PCH "Use precompiled headers" OFF)

if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    set(OLD_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS changed from '${OLD_FLAGS}' to '${CMAKE_CXX_FLAGS}'")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-unused-command-line-argument)
    if (DEMO_CLANG_TRACE)
        add_compile_options(-ftime-trace -ftime-trace-granularity=0)
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(external)

add_subdirectory(code/nstl)
add_subdirectory(code/demo)
add_subdirectory(code/common)
add_subdirectory(code/charming_enum)
add_subdirectory(code/logging)
add_subdirectory(code/vko)
add_subdirectory(code/vkgfx)
add_subdirectory(code/memory)
add_subdirectory(code/platform)
add_subdirectory(code/path)
add_subdirectory(code/fs)
add_subdirectory(code/yyjsoncpp)
add_subdirectory(code/mt)
add_subdirectory(code/editor/assets)
add_subdirectory(code/tiny_ctti)
add_subdirectory(code/tiny_glm)
