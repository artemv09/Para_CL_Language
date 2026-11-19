#include "graphviz.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "ошибка в колличестве переданных аргументов\n";
        return 1;
    }

    try
    {
        const std::string file_name = argv[1];
        Lexer lexer(file_name);
        Parser parser(lexer, file_name);
        const auto program = parser.parse_program();

        Segmantic_Analiz analyzer(file_name);
        analyzer.segmantic_check(program.get());

        dump_graphviz(*program, file_name);

        Interpreter interpreter;
        interpreter.run_code(program.get());
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
