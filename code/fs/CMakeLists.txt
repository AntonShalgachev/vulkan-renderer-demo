add_library(fs
    "include/fs/directory.h"
    "include/fs/file.h"
    
    "src/directory.cpp"
    "src/file.cpp"
)

demo_set_common_properties(fs)

target_include_directories(fs PUBLIC
    "include"
)

target_link_libraries(fs
    nstl
    platform
    path
)
