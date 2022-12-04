FetchContent_Declare(
    stb
    URL https://github.com/nothings/stb/archive/8b5f1f37b5b75829fc72d38e7b5d4bcbf8a26d55.tar.gz # Sep 8
)

FetchDependency(stb)

add_library(stb
    ${stb_SOURCE_DIR}/stb_image.h
    stb.c
)
add_library(stb::stb ALIAS stb)

target_include_directories(stb PUBLIC
    "${stb_SOURCE_DIR}"
)
