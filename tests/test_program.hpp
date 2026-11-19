#ifndef TEST_PROGRAM_HPP_
#define TEST_PROGRAM_HPP_

#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "parser.hpp"

inline std::string test_file_name()
{
    const auto* test = testing::UnitTest::GetInstance() -> current_test_info();
    return std::string(test -> test_suite_name()) + "_" + test -> name() + ".pcl";
}

inline std::unique_ptr<Block_Stmt> parse_program(const std::string& source)
{
    const auto name = test_file_name();
    std::ofstream file(name);
    file << source;
    file.close();
    
    if (!file)
    {
        throw std::logic_error("Не удалось записать " + name);
    }

    Lexer lexer(name);
    std::remove(name.c_str());
    Parser parser(lexer, name);
    return parser.parse_program();
}

class Test_Streams
{
    public:
        std::istringstream input;
        std::ostringstream output;

        Test_Streams():
            old_input(std::cin.rdbuf(input.rdbuf())),
            old_output(std::cout.rdbuf(output.rdbuf()))
        {
        }

        ~Test_Streams()
        {
            std::cin.rdbuf(old_input);
            std::cout.rdbuf(old_output);
        }

    private:
        std::streambuf* old_input;
        std::streambuf* old_output;
};

#endif
