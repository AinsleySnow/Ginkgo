add_library(
    ginkgo_IR
    OBJECT
    Instr.cc
    IRBuilder.cc
    IROperand.cc
    IRType.cc
    Value.cc
)

target_precompile_headers(
    ginkgo_IR
    PRIVATE Instr.h
    PRIVATE IRBuilder.h
    PRIVATE IROperand.h
    PRIVATE IRType.h
    PRIVATE Value.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_IR>
    PARENT_SCOPE)
