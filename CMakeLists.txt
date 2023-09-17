cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Ginkgo VERSION 0.0.1)

# People keep running CMake in the wrong folder, completely nuking their project or creating weird bugs.
# This checks if you're running CMake from a folder that already has CMakeLists.txt.
# Importantly, this catches the common case of running it from the root directory.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)
if (EXISTS "${PATH_TO_CMAKELISTS_TXT}")
    message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(GINKGO_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)
set(GINKGO_THIRD_PARTY_INCLUDE_DIR
    ${PROJECT_SOURCE_DIR}/third_party
    ${PROJECT_SOURCE_DIR}/third_party/fmt/include
    ${PROJECT_SOURCE_DIR}/third_party/utfcpp/source)

set(GINKGO_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

execute_process(
    COMMAND lex -o ${GINKGO_SRC_DIR}/parser/lexer.cc ${GINKGO_SRC_DIR}/parser/lexer.ll
    COMMAND bison ${GINKGO_SRC_DIR}/parser/yacc.yy -dy -Wno-yacc -o ${GINKGO_SRC_DIR}/parser/yacc.cc
)

include_directories(${GINKGO_SRC_INCLUDE_DIR} ${GINKGO_THIRD_PARTY_INCLUDE_DIR})

include(ExternalProject)
ExternalProject_Add(
    gkcpp
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/gkcpp
    CONFIGURE_COMMAND ""
    BUILD_COMMAND
        make -C <SOURCE_DIR> && mv <SOURCE_DIR>/gkcpp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    BUILD_IN_SOURCE FALSE
    INSTALL_COMMAND ""
)

add_custom_command(
    OUTPUT ${GINKGO_SRC_DIR}/parser/lexer.cc
    COMMAND lex -o ${GINKGO_SRC_DIR}/parser/lexer.cc ${GINKGO_SRC_DIR}/parser/lexer.ll
    DEPENDS ${GINKGO_SRC_DIR}/parser/lexer.ll
    COMMENT "Build target lexer"
)

add_custom_command(
    OUTPUT ${GINKGO_SRC_DIR}/parser/yacc.cc
    COMMAND bison ${GINKGO_SRC_DIR}/parser/yacc.yy -dy -Wno-yacc -o ${GINKGO_SRC_DIR}/parser/yacc.cc
    DEPENDS ${GINKGO_SRC_DIR}/parser/yacc.yy
    COMMENT "Build target parser"
)

add_subdirectory(gklib)
add_subdirectory(src)
add_subdirectory(third_party)

add_custom_target(parser DEPENDS ${GINKGO_SRC_DIR}/parser/yacc.cc)
add_custom_target(lexer DEPENDS ${GINKGO_SRC_DIR}/parser/lexer.cc)
add_custom_target(test COMMAND cd ${PROJECT_SOURCE_DIR}/tests && bash test.sh && cd - > /dev/null)
