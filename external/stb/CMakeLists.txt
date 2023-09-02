FetchContent_Declare(
    stb
    URL https://github.com/nothings/stb/archive/5736b15f7ea0ffb08dd38af21067c314d6a3aae9.tar.gz # re-add perlin noise again
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
