add_library(picofmt
    "include/picofmt/config.h"
    "include/picofmt/context.h"
    "include/picofmt/core.h"
    "include/picofmt/formatter.h"
    "include/picofmt/generic_format_spec_parser.h"
    "include/picofmt/picofmt.h"
    "include/picofmt/standard_formatters.h"
    "include/picofmt/writer.h"

    "include/picofmt/detail/context_base.h"
    "include/picofmt/detail/core_impl.h"
    "include/picofmt/detail/generic_format_spec_parser_impl.h"
    "include/picofmt/detail/simple_string_view.h"
    "include/picofmt/detail/standard_formatters_impl.h"
    "include/picofmt/detail/util.h"
    "include/picofmt/detail/writer_base.h"

    "src/core_impl.cpp"
    "src/generic_format_spec_parser_impl.cpp"
    "src/standard_formatters_impl.cpp"
    "src/util.cpp"
)

target_include_directories(picofmt PUBLIC include)
demo_set_common_properties(picofmt)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(picofmt PUBLIC -Wno-unsafe-buffer-usage) # TODO: revisit
    target_compile_options(picofmt PUBLIC -Wno-unused-macros)
    target_compile_options(picofmt PUBLIC -Wno-format-nonliteral)
endif()
