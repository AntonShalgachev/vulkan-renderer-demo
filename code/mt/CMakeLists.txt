add_library(mt
    "include/mt/thread.h"
    "include/mt/mutex.h"
    "include/mt/lock_guard.h"
    "include/mt/thread_id.h"
    "include/mt/atomic.h"
    
    "src/thread.cpp"
    "src/mutex.cpp"
    "src/lock_guard.cpp"
    "src/thread_id.cpp"
)

demo_set_common_properties(mt)

target_include_directories(mt PUBLIC
    "include"
)

target_link_libraries(mt
    nstl
    platform
)
