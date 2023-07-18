add_library(
    ginkgo_pass
    OBJECT
    DUChains.cc
    SimpleAlloc.cc
    x64Alloc.cc
)

target_precompile_headers(
    ginkgo_pass
    PRIVATE DUChains.h
    PRIVATE Pass.h
    PRIVATE Pipeline.h
    PRIVATE SimpleAlloc.h
    PRIVATE x64Alloc.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_pass>
    PARENT_SCOPE)
