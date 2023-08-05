add_library(
    ginkgo_pass
    OBJECT
    Dominators.cc
    DUInfo.cc
    FlowGraph.cc
    Liveness.cc
    LoopAnalyze.cc
    SimpleAlloc.cc
    x64Alloc.cc
)

target_precompile_headers(
    ginkgo_pass
    PRIVATE DUInfo.h
    PRIVATE Dominators.h
    PRIVATE FlowGraph.h
    PRIVATE Liveness.h
    PRIVATE LoopAnalyze.h
    PRIVATE Pass.h
    PRIVATE Pipeline.h
    PRIVATE SimpleAlloc.h
    PRIVATE x64Alloc.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_pass>
    PARENT_SCOPE)
