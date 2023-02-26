cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Ginkgo)

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
include_directories(${GINKGO_SRC_INCLUDE_DIR})

add_subdirectory(src)

set(GINKGO_SRC_DIR ${PROJECT_SOURCE_DIR}/src)

add_custom_target(parser
    COMMAND bison ${GINKGO_SRC_DIR}/parser/yacc.yy -dy -o ${GINKGO_SRC_DIR}/parser/yacc.cc
)

add_custom_target(lexer
    COMMAND lex ${GINKGO_SRC_DIR}/parser/lexer.ll -o ${GINKGO_SRC_DIR}/parser/lexer.cc
)
