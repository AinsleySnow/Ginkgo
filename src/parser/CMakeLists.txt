add_library(
    ginkgo_parser
    OBJECT
    lexer.cc
    yacc.cc
)

target_precompile_headers(
    ginkgo_parser
    PRIVATE yacc.hh
)


set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_parser>
    PARENT_SCOPE)
