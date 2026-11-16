#include "graphviz.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 3 || std::string(argv[1]) != "--dump-ast")
    {
        std::cerr << "Использование: paracl --dump-ast program.pcl\n";
        return 1;
    }
    try
    {
        Lexer_namespace::Lexer lexer(argv[2]);
        Parser parser(lexer, argv[2]);
        const auto program = parser.parse_program();
        bool flag = true;
        const auto path = dump_graphviz(*program, argv[2]);
        Segmantic_Analiz segm_analiz(argv[2]);
        segm_analiz.segmantic_check(program.get(), Assigment_Parent::Non);
        if(flag) {std::cout << "удача" << '\n';}
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
