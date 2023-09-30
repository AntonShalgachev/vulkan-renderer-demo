set(ALL_FILES
    "tiny_ctti.h"
    "fmt.h"
    "fmt.cpp"

    "json-nstl.h"
    "json-nstl.cpp"
    "json-tiny-ctti.h"
    "json-glm.h"
    "json-glm.cpp"
    
    "Timer.cpp"
    "Timer.h"
    "Utils.cpp"
    "Utils.h"

    "align.h"
    "align.cpp"
)

add_library(common ${ALL_FILES})

target_include_directories(common
    PRIVATE
    "${CMAKE_SOURCE_DIR}/include"

    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

demo_set_common_properties(common)

target_compile_definitions(common PRIVATE _CRT_SECURE_NO_WARNINGS) # TODO remove?

target_link_libraries(common
    nstl
    memory
    yyjsoncpp
    fs
    tiny_ctti
    tglm
    picofmt
    mt
)
