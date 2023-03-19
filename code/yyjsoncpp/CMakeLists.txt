add_library(yyjsoncpp
    "include/yyjsoncpp/array_ref.h"
    "include/yyjsoncpp/config.h"
    "include/yyjsoncpp/doc.h"
    "include/yyjsoncpp/object_ref.h"
    "include/yyjsoncpp/serializer.h"
    "include/yyjsoncpp/type.h"
    "include/yyjsoncpp/value_ref.h"
    "include/yyjsoncpp/write_flags.h"
    "include/yyjsoncpp/yyjsoncpp.h"
    "include/yyjsoncpp/default_serializer.h"
    
    "src/array_ref.cpp"
    "src/doc.cpp"
    "src/object_ref.cpp"
    "src/value_ref.cpp"
    "src/default_serializer.cpp"
)

target_compile_features(yyjsoncpp PUBLIC cxx_std_20)

target_include_directories(yyjsoncpp PUBLIC
    "include"
)

target_link_libraries(yyjsoncpp
    yyjson
    nstl # TODO remove
)
