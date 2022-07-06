cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

project(vulkan-renderer-demo LANGUAGES C CXX VERSION 1.0)

set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
    CACHE STRING "" FORCE
)

if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    set(OLD_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS changed from '${OLD_FLAGS}' to '${CMAKE_CXX_FLAGS}'")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

############################### Dependencies ###############################
find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.3.7.tar.gz # 3.3.6
)

FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.tar.gz # 0.9.9.8
)

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.88.tar.gz # 1.88
)

FetchContent_Declare(
    tinygltf
    URL https://github.com/syoyo/tinygltf/archive/3d4150419e8ad40dab89d6f4079382377b492310.zip # Merge pull request #362 from hschwane/master
)

FetchContent_Declare(
    magic_enum
    URL https://github.com/Neargye/magic_enum/archive/refs/tags/v0.8.0.tar.gz # 0.8.0
)

FetchContent_Declare(
    coil
    URL https://github.com/AntonShalgachev/coil/archive/8dc61f58d3a1084257c46b0fd2920c00b8763d65.zip # remove a resolved TODO comment
)

FetchContent_Declare(
    rapidfuzz
    URL https://github.com/maxbachmann/rapidfuzz-cpp/archive/refs/tags/v1.0.4.tar.gz # 1.0.4
)

FetchContent_Declare(
    argparse
    URL https://github.com/p-ranav/argparse/archive/refs/tags/v2.6.tar.gz # 2.6
)

FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz # 1.10.0
)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")
set(GLFW_VULKAN_STATIC ON CACHE INTERNAL "")

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "")
set(TINYGLTF_BUILD_GL_EXAMPLES OFF CACHE INTERNAL "")
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF CACHE INTERNAL "")
set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF CACHE INTERNAL "")
set(TINYGLTF_HEADER_ONLY OFF CACHE INTERNAL "")
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "")

set(COIL_BASIC_TYPENAME OFF CACHE INTERNAL "")

message(STATUS "Fetching glfw...")
FetchContent_MakeAvailable(glfw)
message(STATUS "glfw directory: ${glfw_SOURCE_DIR}")

message(STATUS "Fetching glm...")
FetchContent_MakeAvailable(glm)
message(STATUS "glm directory: ${glm_SOURCE_DIR}")

message(STATUS "Fetching imgui...")
FetchContent_MakeAvailable(imgui)
message(STATUS "imgui directory: ${imgui_SOURCE_DIR}")

message(STATUS "Fetching tinygltf...")
FetchContent_MakeAvailable(tinygltf)
message(STATUS "tinygltf directory: ${tinygltf_SOURCE_DIR}")

message(STATUS "Fetching magic_enum...")
FetchContent_MakeAvailable(magic_enum)
message(STATUS "magic_enum directory: ${magic_enum_SOURCE_DIR}")

message(STATUS "Fetching coil...")
FetchContent_MakeAvailable(coil)
message(STATUS "coil directory: ${coil_SOURCE_DIR}")

message(STATUS "Fetching rapidfuzz...")
FetchContent_MakeAvailable(rapidfuzz)
message(STATUS "rapidfuzz directory: ${rapidfuzz_SOURCE_DIR}")

message(STATUS "Fetching argparse...")
FetchContent_MakeAvailable(argparse)
message(STATUS "argparse directory: ${argparse_SOURCE_DIR}")

message(STATUS "Fetching spdlog...")
FetchContent_MakeAvailable(spdlog)
message(STATUS "spdlog directory: ${spdlog_SOURCE_DIR}")
############################### ############ ###############################

add_subdirectory(code/imgui)
add_subdirectory(code/imgui-backend)
add_subdirectory(code/renderer)
add_subdirectory(code/demo)
