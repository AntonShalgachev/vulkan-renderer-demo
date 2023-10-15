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
    coil
    URL https://github.com/AntonShalgachev/coil/archive/a1365d2aa789675e37ca388c5bcf382502af7c1b.tar.gz # Merge pull request #39 from AntonShalgachev/restructure-compilation-time-benchmark
)
set(COIL_BASIC_TYPENAME OFF CACHE INTERNAL "")
FetchDependency(coil)

FetchContent_Declare(
    yyjson
    URL https://github.com/AntonShalgachev/yyjson/archive/af0f7e0737244f6b545c9e84b5d8394b90f1bbbb.tar.gz # Don't set build types in CMake if yyjson is used as dependency
)
FetchDependency(yyjson)

FetchContent_Declare(
    cglm
    URL https://github.com/recp/cglm/archive/refs/tags/v0.9.1.tar.gz
)
set(CGLM_SHARED OFF CACHE INTERNAL "")
set(CGLM_STATIC ON CACHE INTERNAL "")
FetchDependency(cglm)
set_target_properties(cglm PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
