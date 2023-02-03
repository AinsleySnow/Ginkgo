add_library(
    ginkgo_ast
    OBJECT
    CType.cc
    Declaration.cc
    Expression.cc
    Statement.cc
)

target_precompile_headers(
    ginkgo_ast
    PRIVATE CType.h
    PRIVATE Declaration.h
    PRIVATE Expr.h
    PRIVATE Expression.h
    PRIVATE Statement.h
    PRIVATE Tag.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_ast>
    PARENT_SCOPE)
