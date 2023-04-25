add_library(tglm
    "include/tglm/detail/cglm_types.h"
    
    "include/tglm/types/ivec2.h"
    "include/tglm/types/mat4.h"
    "include/tglm/types/quat.h"
    "include/tglm/types/vec2.h"
    "include/tglm/types/vec3.h"
    "include/tglm/types/vec4.h"

    "include/tglm/affine.h"
    "include/tglm/fwd.h"
    "include/tglm/tglm.h"
    "include/tglm/types.h"
    "include/tglm/util.h"

    "src/affine.cpp"
    "src/ivec2.cpp"
    "src/mat4.cpp"
    "src/quat.cpp"
    "src/util.cpp"
    "src/vec2.cpp"
    "src/vec3.cpp"
    "src/vec4.cpp"
)

target_compile_features(tglm PUBLIC cxx_std_17)

target_include_directories(tglm
    PUBLIC "include"
)

target_link_libraries(tglm cglm_headers)
