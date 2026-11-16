#include "graphviz.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Использование: paracl (--check | --dump-ast | --run) program.pcl\n";
        return 1;
    }
    const std::string mode = argv[1];
    if (mode != "--check" && mode != "--dump-ast" && mode != "--run")
    {
        std::cerr << "Неизвестный режим: " << mode << '\n';
        return 1;
    }
    try
    {
        Lexer_namespace::Lexer lexer(argv[2]);
        Parser parser(lexer, argv[2]);
        const auto program = parser.parse_program();
        Segmantic_Analiz segm_analiz(argv[2]);
        segm_analiz.segmantic_check(program.get(), Assigment_Parent::Non);
        if (mode == "--dump-ast")
        {
            const auto path = dump_graphviz(*program, argv[2]);
            std::cout << path << '\n';
        }
        else if (mode == "--run")
        {
            Interpreter interpreter;
            interpreter.run_code(program.get());
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
