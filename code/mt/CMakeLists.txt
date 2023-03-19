add_library(mt
    "include/mt/thread.h"
    
    "src/thread.cpp"
)

target_compile_features(mt PUBLIC cxx_std_20)

target_include_directories(mt PUBLIC
    "include"
)

target_link_libraries(mt
    nstl
    platform
)
