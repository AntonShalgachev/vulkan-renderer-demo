set(PROJECT_NAME libthirdparty)

set(ALL_FILES
    "lib_implementation.cpp"
)

add_library(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)
