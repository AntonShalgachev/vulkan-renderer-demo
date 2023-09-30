add_library(common
    "include/common/tiny_ctti.h"
    "include/common/fmt.h"
    "include/common/json-nstl.h"
    "include/common/json-tiny-ctti.h"
    "include/common/json-glm.h"
    "include/common/Timer.h"
    "include/common/Utils.h"
    "include/common/align.h"

    "src/fmt.cpp"
    "src/json-nstl.cpp"
    "src/json-glm.cpp"
    "src/Timer.cpp"
    "src/Utils.cpp"
    "src/align.cpp"
)

target_include_directories(common PUBLIC "include")

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
