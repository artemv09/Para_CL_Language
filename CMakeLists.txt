cmake_minimum_required(VERSION 3.16)
project(ParaCL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_RUN_PROGRAM "Build ParaCL executable" ON)

add_library(paracl_frontend
    src/diagnostic.cpp
    src/token.cpp
    src/lexer.cpp
    src/ast.cpp
    graphviz/graphviz.cpp
    src/parser.cpp
    src/semantic_analyzer.cpp
)
target_include_directories(paracl_frontend PUBLIC include graphviz)

add_library(paracl_runtime src/interpreter.cpp)
target_link_libraries(paracl_runtime PUBLIC paracl_frontend)

if(BUILD_RUN_PROGRAM)
    add_executable(paracl src/main.cpp)
    target_link_libraries(paracl PRIVATE paracl_runtime)
endif()

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)

    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()

    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "Use shared MSVC runtime for GoogleTest" FORCE)
    endif()

    FetchContent_Declare(googletest
        URL https://codeload.github.com/google/googletest/tar.gz/refs/tags/v1.14.0
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
    )
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)

    add_executable(paracl_lexer_tests tests/lexer_test.cpp)
    target_link_libraries(paracl_lexer_tests PRIVATE paracl_frontend GTest::gtest_main)
    gtest_discover_tests(paracl_lexer_tests)

    add_executable(paracl_parser_tests tests/parser_test.cpp)
    target_link_libraries(paracl_parser_tests PRIVATE paracl_frontend GTest::gtest_main)
    gtest_discover_tests(paracl_parser_tests)

    add_executable(paracl_semantic_analyzer_tests tests/semantic_analyzer_test.cpp)
    target_link_libraries(paracl_semantic_analyzer_tests PRIVATE paracl_frontend GTest::gtest_main)
    gtest_discover_tests(paracl_semantic_analyzer_tests)

    add_executable(paracl_interpreter_tests tests/interpreter_test.cpp)
    target_link_libraries(paracl_interpreter_tests PRIVATE paracl_runtime GTest::gtest_main)
    gtest_discover_tests(paracl_interpreter_tests PROPERTIES TIMEOUT 5)

    if(TARGET paracl)
        add_executable(paracl_e2e_tests tests/e2e_test.cpp)
        target_link_libraries(paracl_e2e_tests PRIVATE paracl_frontend GTest::gtest_main)
        target_compile_definitions(paracl_e2e_tests PRIVATE
            PARACL_EXECUTABLE="$<TARGET_FILE:paracl>"
            PARACL_E2E_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/e2e"
        )
        add_dependencies(paracl_e2e_tests paracl)
        gtest_discover_tests(paracl_e2e_tests PROPERTIES TIMEOUT 5)
    endif()
endif()
