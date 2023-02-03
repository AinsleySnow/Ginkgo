add_library(
    ginkgo_messages
    OBJECT
    errmsg-en_us.cc
    Error.cc
)

target_precompile_headers(
    ginkgo_messages
    PRIVATE Error.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_messages>
    PARENT_SCOPE)
