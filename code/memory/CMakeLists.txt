add_library(memory
    "include/memory/untracked_allocator.h"
    "include/memory/tracking.h"
    
    "src/untracked_allocator.cpp"
    "src/tracking.cpp"
    "src/new.cpp"
)

target_compile_features(memory PUBLIC cxx_std_20)

target_include_directories(memory PUBLIC
    "include"
)

target_link_libraries(memory
    nstl
    platform
    logging
)
