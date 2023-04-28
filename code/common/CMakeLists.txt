set(PROJECT_NAME common)

set(ALL_FILES
    "charming_enum.h"
    "tiny_ctti.h"
    "fmt.h"

    "json-nstl.h"
    "json-nstl.cpp"
    "json-tiny-ctti.h"
    "json-glm.h"
    "json-glm.cpp"
    
    "Timer.cpp"
    "Timer.h"
    "Utils.cpp"
    "Utils.h"
)

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/include"

    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS) # TODO remove?

target_link_libraries(${PROJECT_NAME}
    nstl
    charming_enum
    fmt::fmt
    memory
    yyjsoncpp
    fs
    tiny_ctti
    tglm
)
