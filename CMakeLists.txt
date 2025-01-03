cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(vulkan-renderer-demo LANGUAGES C CXX VERSION 1.0)

# TODO get rid of all "demo" names

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()

option(DEMO_CLANG_TRACE "Generate Clang trace files" OFF)
option(DEMO_PCH "Use precompiled headers" OFF)

macro(demo_set_common_properties name)
    target_compile_features(${name} PUBLIC cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(${name} PRIVATE -Werror)
      # target_compile_options(${name} PRIVATE -Wall)
      # target_compile_options(${name} PRIVATE -Wextra -Wpedantic)
      target_compile_options(${name} PRIVATE -Wno-c++98-compat-pedantic)
      target_compile_options(${name} PRIVATE -Wno-shadow-field-in-constructor)
      target_compile_options(${name} PRIVATE -Wno-shadow-uncaptured-local)
      target_compile_options(${name} PRIVATE -Wno-implicit-int-float-conversion)
      target_compile_options(${name} PRIVATE -Wno-double-promotion)
      target_compile_options(${name} PRIVATE -Wno-global-constructors)
      target_compile_options(${name} PRIVATE -Wno-exit-time-destructors)
      # target_compile_options(${name} PRIVATE -Wno-error=unused-variable)
    elseif(MSVC)
      target_compile_options(${name} PRIVATE /W4 /WX)
    endif()

    if (MSVC AND (CMAKE_GENERATOR MATCHES "Visual Studio"))
      target_compile_options(${name} PRIVATE /MP) # enable parallel build within Visual Studio
    endif()
endmacro()

function(target_mark_includes_system target)
  set(libs ${ARGN})
  foreach(lib ${libs})
    get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${target} SYSTEM PRIVATE ${lib_include_dirs})
  endforeach(lib)
endfunction(target_mark_includes_system)

if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    set(OLD_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS changed from '${OLD_FLAGS}' to '${CMAKE_CXX_FLAGS}'")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (DEMO_CLANG_TRACE)
        add_compile_options(-ftime-trace -ftime-trace-granularity=0)
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(external)

add_subdirectory(code/nstl)
add_subdirectory(code/demo)
add_subdirectory(code/common)
add_subdirectory(code/logging)
# add_subdirectory(code/vko)
add_subdirectory(code/memory)
add_subdirectory(code/platform)
add_subdirectory(code/platform_win64)
add_subdirectory(code/platform_clang)
add_subdirectory(code/platform_msvc)
add_subdirectory(code/path)
add_subdirectory(code/fs)
add_subdirectory(code/yyjsoncpp)
add_subdirectory(code/mt)
add_subdirectory(code/editor)
add_subdirectory(code/tiny_ctti)
add_subdirectory(code/tglm)
add_subdirectory(code/picofmt)
add_subdirectory(code/tiny_ktx)
add_subdirectory(code/gfx)
add_subdirectory(code/gfx_vk)
add_subdirectory(code/gfx_vk_win64)
