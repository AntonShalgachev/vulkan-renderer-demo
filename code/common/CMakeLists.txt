set(PROJECT_NAME common)

set(ALL_FILES
    "glm.h"
    "glm.cpp"
    
    "Hash.cpp"
    "Hash.h"
    "Timer.cpp"
    "Timer.h"
    "Utils.cpp"
    "Utils.h"
)

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${CMAKE_SOURCE_DIR}/include"

    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_compile_options(${PROJECT_NAME} PRIVATE ${BASE_COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME}
    glm::glm
)
