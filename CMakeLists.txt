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
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        7d5a16ce714f0b5f4efa3262de22e4d948851525 # 3.3.6
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        c71a50deb5ddf1ea386b91e60fa2e4a26d080074 # 1.87
)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf
    GIT_TAG        17e6e310ce3cd43d7895be3c0116908d1afa8a91 # Merge pull request #341 from mushrom/loopfix
)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG        3d1f6a5a2a3fbcba077e00ad0ccc2dd9fefc2ca7 # 0.7.3
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

message(STATUS "Fetching glfw...")
FetchContent_MakeAvailable(glfw)
message(STATUS "glfw directory: ${glfw_SOURCE_DIR}")

message(STATUS "Fetching glm...")
FetchContent_MakeAvailable(glm)
message(STATUS "glm directory: ${glm_SOURCE_DIR}")

message(STATUS "Fetching imgui...")
FetchContent_MakeAvailable(imgui)
message(STATUS "imgui directory: ${imgui_SOURCE_DIR}")
add_subdirectory(imgui)

message(STATUS "Fetching tinygltf...")
FetchContent_MakeAvailable(tinygltf)
message(STATUS "tinygltf directory: ${tinygltf_SOURCE_DIR}")

message(STATUS "Fetching magic_enum...")
FetchContent_MakeAvailable(magic_enum)
message(STATUS "magic_enum directory: ${magic_enum_SOURCE_DIR}")
############################### ############ ###############################

add_subdirectory(imgui-backend)
add_subdirectory(renderer)
add_subdirectory(demo)
