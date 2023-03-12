add_library(path
    "include/path/path.h"
    "include/path/walker.h"
    
    "src/path.cpp"
    "src/walker.cpp"
)

target_compile_features(path PUBLIC cxx_std_20)

target_include_directories(path PUBLIC
    "include"
)

target_link_libraries(path
    nstl
)
