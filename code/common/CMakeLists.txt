add_library(common
    "include/common/tiny_ctti.h"
    "include/common/fmt.h"
    "include/common/json-nstl.h"
    "include/common/json-tiny-ctti.h"
    "include/common/json-glm.h"
    "include/common/Timer.h"
    "include/common/Utils.h"

    "src/fmt.cpp"
    "src/json-nstl.cpp"
    "src/json-glm.cpp"
    "src/Timer.cpp"
    "src/Utils.cpp"
)

target_include_directories(common PUBLIC "include")

demo_set_common_properties(common)

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
