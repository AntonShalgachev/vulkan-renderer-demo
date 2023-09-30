add_library(memory
    "include/memory/untracked_allocator.h"
    "include/memory/tracking.h"
    "include/memory/memory.h"
    
    "src/untracked_allocator.cpp"
    "src/tracking.cpp"
    "src/new.cpp"
    "src/memory.cpp"
)

demo_set_common_properties(memory)

target_include_directories(memory PUBLIC
    "include"
)

target_link_libraries(memory
    nstl
    platform
    logging
)
