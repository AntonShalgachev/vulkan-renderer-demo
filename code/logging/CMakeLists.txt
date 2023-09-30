add_library(logging
    "logging.h"
    "logging.cpp"
)

demo_set_common_properties(logging)

target_link_libraries(logging
    nstl
    common
    memory
)
