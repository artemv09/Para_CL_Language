cmake_minimum_required(VERSION 3.16)
project(ParaCL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(paracl_frontend
    src/diagnostic.cpp
    src/token.cpp
    src/lexer.cpp
    src/ast.cpp
    src/graphviz.cpp
    src/parser.cpp
    src/semantic_analyzer.cpp
)
target_include_directories(paracl_frontend PUBLIC include)

add_executable(paracl src/main.cpp)
target_link_libraries(paracl PRIVATE paracl_frontend)

include(CTest)
if(BUILD_TESTING)
    add_executable(lexer_test tests/lexer_test.cpp)
    target_link_libraries(lexer_test PRIVATE paracl_frontend)
    add_test(NAME lexer COMMAND lexer_test ${CMAKE_CURRENT_BINARY_DIR}/lexer_input.pcl)

    add_executable(parser_test tests/parser_test.cpp)
    target_link_libraries(parser_test PRIVATE paracl_frontend)
    add_test(NAME parser COMMAND parser_test ${CMAKE_CURRENT_BINARY_DIR}/parser_input.pcl)

    add_executable(semantic_analyzer_test tests/semantic_analyzer_test.cpp)
    target_link_libraries(semantic_analyzer_test PRIVATE paracl_frontend)
    add_test(NAME semantic_analyzer COMMAND semantic_analyzer_test ${CMAKE_CURRENT_BINARY_DIR}/semantic_input.pcl)
endif()
