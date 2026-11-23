cmake_minimum_required(VERSION 3.16...3.24)
project(ParaCL LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_RUN_PROGRAM "Build ParaCL executable" ON)
include(CTest)

add_library(paracl_core
    src/diagnostic.cpp
    src/token.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/semantic_analyzer.cpp
    src/interpreter.cpp
    graphviz/graphviz.cpp
)

target_compile_features(paracl_core PUBLIC cxx_std_20)
target_include_directories(paracl_core PUBLIC include graphviz)

if(BUILD_RUN_PROGRAM)
    add_executable(paracl src/main.cpp)
    target_link_libraries(paracl PRIVATE paracl_core)
endif()

if(NOT BUILD_TESTING)
    return()
endif()

include(FetchContent)

set(BUILD_GMOCK OFF)
if(MSVC)
    set(gtest_force_shared_crt ON)
endif()

FetchContent_Declare(googletest
    URL https://codeload.github.com/google/googletest/tar.gz/refs/tags/v1.14.0
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

function(add_paracl_test name)
    set(target paracl_${name}_tests)

    add_executable(${target} tests/${name}_test.cpp)
    target_link_libraries(${target} PRIVATE paracl_core GTest::gtest_main)
    gtest_discover_tests(${target})
endfunction()

add_paracl_test(lexer)
add_paracl_test(parser)
add_paracl_test(semantic_analyzer)
add_paracl_test(interpreter)

if(TARGET paracl)
    add_paracl_test(e2e)

    target_compile_definitions(paracl_e2e_tests PRIVATE
        "PARACL_EXECUTABLE=\"$<TARGET_FILE:paracl>\""
        "PARACL_E2E_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/tests/e2e\""
    )

    add_dependencies(paracl_e2e_tests paracl)
endif()