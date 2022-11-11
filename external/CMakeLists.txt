include(FetchContent)

macro(FetchDependency name)
    message(STATUS "Fetching ${name}...")
    FetchContent_MakeAvailable(${name})
    message(STATUS "${name} directory: ${${name}_SOURCE_DIR}")
endmacro()

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")

add_subdirectory(imgui)
add_subdirectory(dds-ktx)

FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.3.8.tar.gz # 3.3.8
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")
set(GLFW_VULKAN_STATIC ON CACHE INTERNAL "")
FetchDependency(glfw)

FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.tar.gz # 0.9.9.8
)
FetchDependency(glm)

FetchContent_Declare(
    tinygltf
    URL https://github.com/syoyo/tinygltf/archive/3d4150419e8ad40dab89d6f4079382377b492310.tar.gz # Merge pull request #362 from hschwane/master
)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "")
set(TINYGLTF_BUILD_GL_EXAMPLES OFF CACHE INTERNAL "")
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF CACHE INTERNAL "")
set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF CACHE INTERNAL "")
set(TINYGLTF_HEADER_ONLY OFF CACHE INTERNAL "")
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "")
FetchDependency(tinygltf)

FetchContent_Declare(
    magic_enum
    URL https://github.com/Neargye/magic_enum/archive/refs/tags/v0.8.0.tar.gz # 0.8.0
)
FetchDependency(magic_enum)

FetchContent_Declare(
    coil
    URL https://github.com/AntonShalgachev/coil/archive/2c628391fdf8619aa0391a4c90127d616e74dc74.tar.gz # Merge branch 'master' into stl-fuck-you
)
set(COIL_BASIC_TYPENAME OFF CACHE INTERNAL "")
FetchDependency(coil)

FetchContent_Declare(
    rapidfuzz
    URL https://github.com/maxbachmann/rapidfuzz-cpp/archive/refs/tags/v1.0.4.tar.gz # 1.0.4
)
FetchDependency(rapidfuzz)

FetchContent_Declare(
    argparse
    URL https://github.com/p-ranav/argparse/archive/refs/tags/v2.9.tar.gz # 2.9
)
FetchDependency(argparse)

FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz # 1.10.0
)
FetchDependency(spdlog)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.10.5.tar.gz # 3.10.5
)
FetchDependency(json)

FetchContent_Declare(
    vma
    URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.0.1.tar.gz # 3.0.1
)
FetchDependency(vma)

#Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    eastl
    GIT_REPOSITORY https://github.com/electronicarts/EASTL
    GIT_TAG e8bdd5531ed79c30ccef2fd71e070f5ab9f1222a # 3.18.00
    
    GIT_SUBMODULES "test/packages/EABase"
    GIT_SUBMODULES_RECURSE FALSE
    #GIT_PROGRESS TRUE
)
FetchDependency(eastl)
