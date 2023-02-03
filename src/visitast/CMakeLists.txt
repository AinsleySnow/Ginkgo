add_library(
    ginkgo_visitast
    OBJECT
    CodeChk.cc
    IRGen.cc
    Scope.cc
)


target_precompile_headers(
    ginkgo_visitast
    PRIVATE CodeChk.h
    PRIVATE Identifier.h
    PRIVATE IRGen.h
    PRIVATE Scope.h
    PRIVATE Visitor.h
)


set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_visitast>
    PARENT_SCOPE)
