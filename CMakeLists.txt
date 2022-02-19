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

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")
set(GLFW_VULKAN_STATIC ON CACHE INTERNAL "")

message(STATUS "Fetching glfw...")
FetchContent_MakeAvailable(glfw)
message(STATUS "Fetching glm...")
FetchContent_MakeAvailable(glm)
############################### ############ ###############################

add_subdirectory(imgui)
add_subdirectory(renderer)
add_subdirectory(demo)
add_subdirectory(third_party)
