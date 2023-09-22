add_library(mt
    "include/mt/thread.h"
    "include/mt/mutex.h"
    "include/mt/lock_guard.h"
    
    "src/thread.cpp"
    "src/mutex.cpp"
    "src/lock_guard.cpp"
)

target_compile_features(mt PUBLIC cxx_std_20)

target_include_directories(mt PUBLIC
    "include"
)

target_link_libraries(mt
    nstl
    platform
)
