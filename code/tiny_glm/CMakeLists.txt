add_library(tiny_glm
    "include/tiny_glm/detail/cglm_types.h"
    
    "include/tiny_glm/types/ivec2.h"
    "include/tiny_glm/types/mat4.h"
    "include/tiny_glm/types/quat.h"
    "include/tiny_glm/types/vec2.h"
    "include/tiny_glm/types/vec3.h"
    "include/tiny_glm/types/vec4.h"

    "include/tiny_glm/affine.h"
    "include/tiny_glm/fwd.h"
    "include/tiny_glm/tiny_glm.h"
    "include/tiny_glm/types.h"
    "include/tiny_glm/util.h"

    "src/affine.cpp"
    "src/ivec2.cpp"
    "src/mat4.cpp"
    "src/quat.cpp"
    "src/util.cpp"
    "src/vec2.cpp"
    "src/vec3.cpp"
    "src/vec4.cpp"
)

target_include_directories(tiny_glm
    PUBLIC "include"
)

target_link_libraries(tiny_glm cglm_headers)
