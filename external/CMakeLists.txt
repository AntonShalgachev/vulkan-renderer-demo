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
add_subdirectory(cgltf)
add_subdirectory(stb)
add_subdirectory(glfw)

FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.tar.gz # 0.9.9.8
)
FetchDependency(glm)

FetchContent_Declare(
    coil
    URL https://github.com/AntonShalgachev/coil/archive/32589744ab28925630a51e1e6f28964838632a9f.tar.gz # Merge pull request #35 from AntonShalgachev/fix-dtor-name 
)
set(COIL_BASIC_TYPENAME OFF CACHE INTERNAL "")
FetchDependency(coil)

FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.tar.gz # 9.1.0
)
FetchDependency(fmt)

FetchContent_Declare(
    vma
    URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.0.1.tar.gz # 3.0.1
)
FetchDependency(vma)

FetchContent_Declare(
    yyjson
    URL https://github.com/ibireme/yyjson/archive/refs/tags/0.5.1.tar.gz # 0.5.1
)
FetchDependency(yyjson)
