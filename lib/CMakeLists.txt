add_library(processing INTERFACE processing.hpp)

target_compile_options(processing INTERFACE
    -Wall
    -Wextra
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wpedantic
    -Werror
    -Wdeprecated
)