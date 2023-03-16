add_library(fs
    "include/fs/directory.h"
    "include/fs/file.h"
    
    "src/directory.cpp"
    "src/file.cpp"
)

target_compile_features(fs PUBLIC cxx_std_20)

target_include_directories(fs PUBLIC
    "include"
)

target_link_libraries(fs
    nstl
    platform
    path
)
