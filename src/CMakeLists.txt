add_subdirectory(ast)
add_subdirectory(IR)
add_subdirectory(main)
add_subdirectory(messages)
add_subdirectory(parser)
add_subdirectory(visitast)

add_executable(Ginkgo ${ALL_OBJECT_FILES})
