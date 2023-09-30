add_library(logging
    "include/logging/logging.h"

    "src/logging.cpp"
)

target_include_directories(logging PUBLIC
    "include"
)

demo_set_common_properties(logging)

target_link_libraries(logging
    nstl
    common
    memory
)
