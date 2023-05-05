add_library(
    ginkgo_pass
    OBJECT
    SimpleAlloc.cc
    x64Alloc.cc
)

target_precompile_headers(
    ginkgo_pass
    PRIVATE Pass.h
    PRIVATE Pipeline.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_pass>
    PARENT_SCOPE)
