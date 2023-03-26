add_library(tiny_ctti INTERFACE
    "include/tiny_ctti/tiny_ctti.hpp"
)

target_compile_features(tiny_ctti INTERFACE cxx_std_20)

target_include_directories(tiny_ctti INTERFACE
    "include"
)
