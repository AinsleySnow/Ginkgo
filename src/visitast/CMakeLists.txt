add_library(
    ginkgo_visitast
    OBJECT
    CodeChk.cc
    IRGen.cc
    Scope.cc
    Evaluator.cc
    TypeBuilder.cc
)


target_precompile_headers(
    ginkgo_visitast
    PRIVATE CodeChk.h
    PRIVATE Evaluator.h
    PRIVATE Identifier.h
    PRIVATE IRGen.h
    PRIVATE Scope.h
    PRIVATE ASTVisitor.h
    PRIVATE TypeBuilder.h
)


set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_visitast>
    PARENT_SCOPE)
