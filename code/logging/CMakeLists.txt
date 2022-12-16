add_library(logging
    "logging.h"
    "logging.cpp"
)

target_compile_features(logging PUBLIC cxx_std_20)

target_link_libraries(logging
    nstl
    common
)
