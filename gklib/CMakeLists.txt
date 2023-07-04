set(CMAKE_BUILD_TYPE MinSizeRel)
add_compile_options(-march=native)

add_library(
    gklib
    STATIC
    gkassert.c
    gkbit.c
)

set_target_properties(gklib PROPERTIES PREFIX "")
