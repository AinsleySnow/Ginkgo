add_library(
    ginkgo_main
    OBJECT
    Driver.cc
    main.cc
)

target_precompile_headers(
    ginkgo_main
    PRIVATE Driver.h
)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:ginkgo_main>
    PARENT_SCOPE)
