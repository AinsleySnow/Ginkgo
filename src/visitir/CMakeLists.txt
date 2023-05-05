add_library(
    ginkgo_visitir
    OBJECT
    CodeGen.cc
    EmitAsm.cc
    x64.cc
)


target_precompile_headers(
    ginkgo_visitir
    PRIVATE CodeGen.h
    PRIVATE EmitAsm.h
    PRIVATE IRVisitor.h
)


set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_visitir>
    PARENT_SCOPE)
