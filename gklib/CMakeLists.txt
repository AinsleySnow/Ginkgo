set(CMAKE_BUILD_TYPE MinSizeRel)
add_compile_options(-march=native)

add_library(
    gkcommon
    STATIC
    gkarg.c
    gkassert.c
)

add_library(
    gkc23
    STATIC
    gkbit.c
    gkckdint.c
)

set_target_properties(gkcommon PROPERTIES PREFIX "")
set_target_properties(gkc23 PROPERTIES PREFIX "")
