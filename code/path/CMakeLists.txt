add_library(path
    "include/path/path.h"
    "include/path/walker.h"
    
    "src/path.cpp"
    "src/walker.cpp"
)

demo_set_common_properties(path)

target_include_directories(path PUBLIC
    "include"
)

target_link_libraries(path
    nstl
)
