set(NSTL_ENABLE_ASSERTS OFF CACHE BOOL "Configure NSTL to enable asserts")

add_library(nstl # custom template library
    "include/nstl/Algorithm.h"
    "include/nstl/Assert.h"
    "include/nstl/Buffer.h"
    "include/nstl/Config.h"
    "include/nstl/Expected.h"
    "include/nstl/Hash.h"
    "include/nstl/New.h"
    "include/nstl/Optional.h"
    "include/nstl/Sequence.h"
    "include/nstl/String.h"
    "include/nstl/StringView.h"
    "include/nstl/TypeTraits.h"
    "include/nstl/Unexpected.h"
    "include/nstl/UniquePtr.h"
    "include/nstl/UnorderedMap.h"
    "include/nstl/Utility.h"
    "include/nstl/Vector.h"

    "src/String.cpp"
    "src/StringView.cpp"
    "src/New.cpp"
    "src/Buffer.cpp"
)

target_include_directories(nstl
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(nstl PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(nstl PRIVATE /W4 /WX)
else()
  target_compile_options(nstl PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if (${NSTL_ENABLE_ASSERTS})
    target_compile_definitions(nstl PUBLIC NSTL_CONFIG_ENABLE_ASSERTS=1)
endif()
