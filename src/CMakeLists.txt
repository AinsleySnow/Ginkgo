add_subdirectory(ast)
add_subdirectory(IR)
add_subdirectory(main)
add_subdirectory(messages)
add_subdirectory(parser)
add_subdirectory(pass)
add_subdirectory(visitast)
add_subdirectory(visitir)

add_executable(Ginkgo ${ALL_OBJECT_FILES})

# since utfcpp is just headers
target_link_libraries(Ginkgo fmt)
