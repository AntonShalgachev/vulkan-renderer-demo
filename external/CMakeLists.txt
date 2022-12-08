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
    magic_enum
    URL https://github.com/Neargye/magic_enum/archive/refs/tags/v0.8.0.tar.gz # 0.8.0
)
FetchDependency(magic_enum)

FetchContent_Declare(
    coil
    URL https://github.com/AntonShalgachev/coil/archive/32589744ab28925630a51e1e6f28964838632a9f.tar.gz # Merge pull request #35 from AntonShalgachev/fix-dtor-name 
)
set(COIL_BASIC_TYPENAME OFF CACHE INTERNAL "")
FetchDependency(coil)

FetchContent_Declare(
    rapidfuzz
    URL https://github.com/maxbachmann/rapidfuzz-cpp/archive/refs/tags/v1.0.4.tar.gz # 1.0.4
)
FetchDependency(rapidfuzz)

FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz # 1.10.0
)
set(SPDLOG_ENABLE_PCH ON CACHE INTERNAL "")
FetchDependency(spdlog)

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
