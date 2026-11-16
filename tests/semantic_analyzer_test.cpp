#include "semantic_analyzer.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

void require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

std::unique_ptr<Block_Stmt> parse(const std::string& path, const std::string& source)
{
    {
        std::ofstream file(path, std::ios::binary);
        file << source;
        require(static_cast<bool>(file), "Cannot write test input");
    }
    Lexer_namespace::Lexer lexer(path);
    Parser parser(lexer, path);
    return parser.parse_program();
}

void expect_success(Segmantic_Analiz& analyzer, const std::string& path, const std::string& source)
{
    const auto program = parse(path, source);
    analyzer.segmantic_check(program.get());
}

void expect_error(Segmantic_Analiz& analyzer, const std::string& path, const std::string& source,
                  const std::string& name, const std::string& position)
{
    const auto program = parse(path, source);
    try
    {
        analyzer.segmantic_check(program.get());
    }
    catch(const std::runtime_error& error)
    {
        const std::string expected = path + ":" + position + ": ошибка: переменная '" + name + "' не объявлена";
        require(error.what() == expected, "Wrong semantic diagnostic: " + std::string(error.what()));
        return;
    }
    throw std::runtime_error("Undeclared variable accepted: " + source);
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc == 2, "Pass a temporary input file path");
        const std::string path = argv[1];
        Segmantic_Analiz analyzer(path);

        expect_success(analyzer, path, "");
        expect_success(analyzer, path, "{}");
        expect_success(analyzer, path, "x = 1; print x;");
        expect_error(analyzer, path, "print x;", "x", "1:7");
        expect_error(analyzer, path, "x = x + 1;", "x", "1:5");
        expect_success(analyzer, path, "if (1) {}");
        expect_error(analyzer, path, "{ x = 1; } print x;", "x", "1:18");
        expect_success(analyzer, path, "x = 1; { x = x + 1; } print x;");
        expect_success(analyzer, path, "{ x = 1; { x = x + 1; } print x; }");
        expect_error(analyzer, path, "print -x;", "x", "1:8");
        expect_error(analyzer, path, "print x + 1;", "x", "1:7");
        expect_error(analyzer, path, "print 1 + x;", "x", "1:11");

        expect_success(analyzer, path, "x = ?; while (x) { x = x - 1; } print x;");
        expect_success(analyzer, path, "if (1) {} else if (2) {} else {}");
        expect_error(analyzer, path, "if (x) {}", "x", "1:5");
        expect_error(analyzer, path, "if (1) {} else { print x; }", "x", "1:24");
        expect_error(analyzer, path, "while (x) {}", "x", "1:8");
        expect_error(analyzer, path, "while (1) { print x; }", "x", "1:19");

        const std::string failed_source = "outer = 1; { inner = 2; { print missing; } }";
        expect_error(analyzer, path, failed_source, "missing",
                     "1:" + std::to_string(failed_source.find("missing") + 1));
        expect_error(analyzer, path, "print outer;", "outer", "1:7");
        expect_error(analyzer, path, "print inner;", "inner", "1:7");
        expect_success(analyzer, path, "outer = 1; { inner = outer + 1; print inner; }");
        expect_error(analyzer, path, "print outer;", "outer", "1:7");

        const auto program = parse(path, "x = 1; { x = x + 1; } print x;");
        analyzer.segmantic_check(program.get());
        analyzer.segmantic_check(program.get());

        std::cout << "Semantic analyzer tests passed\n";
        return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
