cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(vulkan-renderer-demo LANGUAGES C CXX VERSION 1.0)

set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
    CACHE STRING "" FORCE
)

set(DEMO_CLANG_TRACE OFF CACHE BOOL "Generate Clang trace files")

if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    set(OLD_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS changed from '${OLD_FLAGS}' to '${CMAKE_CXX_FLAGS}'")
endif()

set(BASE_COMPILE_OPTIONS)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND BASE_COMPILE_OPTIONS -Wno-unused-command-line-argument)

    if (DEMO_CLANG_TRACE)
        list(APPEND BASE_COMPILE_OPTIONS -ftime-trace -ftime-trace-granularity=0)
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(external)
add_subdirectory(code/nstl)
add_subdirectory(code/renderer)
add_subdirectory(code/demo)
add_subdirectory(code/common)
