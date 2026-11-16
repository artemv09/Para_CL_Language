#include "parser.hpp"
#include "graphviz.hpp"

#include <vector>

#include <fstream>
#include <iostream>
#include <sstream>
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

void expect_tree(const std::string& path, const std::string& source, const std::string& expected)
{
    const auto program = parse(path, source);
    std::ostringstream output;
    Graphviz_Writer(output).write(*program);
    // Convert the expected indented tree to DOT independently of the AST traversal.
    std::ostringstream nodes;
    nodes << "digraph AST {\n  node [shape=box];\n  ordering=out;\n";
    std::istringstream lines(expected);
    std::string line;
    std::vector<std::size_t> ids;
    std::size_t id = 0;
    while (std::getline(lines, line))
    {
        const auto depth = line.find_first_not_of(' ') / 2;
        while (ids.size() > depth)
        {
            const auto child = ids.back();
            ids.pop_back();
            if (!ids.empty())
                nodes << "  node" << ids.back() << " -> node" << child << ";\n";
        }
        const auto label = line.substr(depth * 2);
        const auto separator = label.find(' ');
        const auto type = label.substr(0, separator);
        std::string value;
        if (separator != std::string::npos)
            value = label.substr(separator + 1);
        if (type == "Assignment_Stmt") value = "=";
        else if (type == "Input_Expr") value = "?";
        else if (type == "Print_Stmt") value = "print";
        else if (type == "Block_Stmt") value = "{}";
        else if (type == "If_Stmt") value = "if";
        else if (type == "While_Stmt") value = "while";
        nodes << "  node" << id << " [label=\"" << type << "\\n" << value << "\"];\n";
        ids.push_back(id++);
    }
    while (ids.size() > 1)
    {
        const auto child = ids.back();
        ids.pop_back();
        nodes << "  node" << ids.back() << " -> node" << child << ";\n";
    }
    nodes << "}\n";
    require(output.str() == nodes.str(), "Wrong AST for: " + source + "\n" + output.str());
}

void expect_error(const std::string& path, const std::string& source,
                  const std::string& position, const std::string& expected,
                  const std::string& received)
{
    try
    {
        parse(path, source);
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();
        require(message.find(path + ":" + position + ": ошибка:") == 0,
                "Wrong error position: " + message);
        require(message.find("ожидалось " + expected) != std::string::npos,
                "Missing expected element: " + message);
        require(message.find(", получен " + received) != std::string::npos,
                "Missing received token: " + message);
        return;
    }
    throw std::runtime_error("Invalid input accepted: " + source);
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc == 2, "Pass a temporary input file path");
        const std::string path = argv[1];
        expect_tree(path, "", "Block_Stmt\n");
        expect_tree(path, "x = 42;", "Block_Stmt\n  Assignment_Stmt\n    Variable_Expr x\n    Integer_Expr 42\n");
        expect_tree(path, "x = ?; print x;",
                    "Block_Stmt\n  Assignment_Stmt\n    Variable_Expr x\n    Input_Expr\n  Print_Stmt\n    Variable_Expr x\n");
        expect_tree(path, "print 2 + 3 * 4;",
                    "Block_Stmt\n  Print_Stmt\n    Binary_Expr +\n      Integer_Expr 2\n"
                    "      Binary_Expr *\n        Integer_Expr 3\n        Integer_Expr 4\n");
        expect_tree(path, "x = (2 + 3) * 4;",
                    "Block_Stmt\n  Assignment_Stmt\n    Variable_Expr x\n    Binary_Expr *\n      Binary_Expr +\n"
                    "        Integer_Expr 2\n        Integer_Expr 3\n      Integer_Expr 4\n");
        expect_tree(path, "print 10 - 3 - 2;",
                    "Block_Stmt\n  Print_Stmt\n    Binary_Expr -\n      Binary_Expr -\n"
                    "        Integer_Expr 10\n        Integer_Expr 3\n      Integer_Expr 2\n");
        expect_tree(path, "print -+(-(2));",
                    "Block_Stmt\n  Print_Stmt\n    Unary_Expr -\n      Unary_Expr +\n"
                    "        Unary_Expr -\n          Integer_Expr 2\n");
        expect_tree(path, "print 1 < 2 <= 3 == 4 != 5;",
                    "Block_Stmt\n  Print_Stmt\n    Binary_Expr !=\n      Binary_Expr ==\n"
                    "        Binary_Expr <=\n          Binary_Expr <\n            Integer_Expr 1\n"
                    "            Integer_Expr 2\n          Integer_Expr 3\n        Integer_Expr 4\n"
                    "      Integer_Expr 5\n");
        expect_tree(path, "print 8 / 2 % 3; print 3 > 2 >= 1;",
                    "Block_Stmt\n  Print_Stmt\n    Binary_Expr %\n      Binary_Expr /\n"
                    "        Integer_Expr 8\n        Integer_Expr 2\n      Integer_Expr 3\n"
                    "  Print_Stmt\n    Binary_Expr >=\n      Binary_Expr >\n"
                    "        Integer_Expr 3\n        Integer_Expr 2\n      Integer_Expr 1\n");
        expect_tree(path, "{} {{}}", "Block_Stmt\n  Block_Stmt\n  Block_Stmt\n    Block_Stmt\n");
        expect_tree(path, "if (x) {}", "Block_Stmt\n  If_Stmt\n    Variable_Expr x\n    Block_Stmt\n");
        expect_tree(path, "if (x) {} else {}",
                    "Block_Stmt\n  If_Stmt\n    Variable_Expr x\n    Block_Stmt\n    Block_Stmt\n");
        const std::string chain = "Block_Stmt\n  If_Stmt\n    Integer_Expr 1\n    Block_Stmt\n"
                                  "    If_Stmt\n      Integer_Expr 2\n      Block_Stmt\n"
                                  "      If_Stmt\n        Integer_Expr 3\n        Block_Stmt\n";
        expect_tree(path, "if(1){}else if(2){}else if(3){}", chain);
        expect_tree(path, "if(1){}else if(2){}else if(3){}else{}", chain + "        Block_Stmt\n");
        expect_tree(path, "while(x){if(y){while(z){}}else{}}",
                    "Block_Stmt\n  While_Stmt\n    Variable_Expr x\n    Block_Stmt\n"
                    "      If_Stmt\n        Variable_Expr y\n        Block_Stmt\n"
                    "          While_Stmt\n            Variable_Expr z\n            Block_Stmt\n"
                    "        Block_Stmt\n");
        expect_tree(path, "print 9223372036854775807;",
                    "Block_Stmt\n  Print_Stmt\n    Integer_Expr 9223372036854775807\n");

        const auto program = parse(path, " \n{\n x = -2 + ?;\n print x;\n}");
        require(program->location.line == 1 && program->location.column == 1 &&
                program->location.offset == 0, "Wrong root position");
        const auto* block = dynamic_cast<Block_Stmt*>(program->statements.at(0).get());
        require(block && block->location.line == 2 && block->location.column == 1 &&
                block->location.offset == 2, "Wrong block position");
        const auto* assignment = dynamic_cast<Assignment_Stmt*>(block->statements.at(0).get());
        require(assignment && assignment->target && assignment->target->name == "x" && assignment->location.line == 3 &&
                assignment->location.column == 2 && assignment->location.offset == 5,
                "Wrong assignment position");
        require(assignment->target->location.line == 3 && assignment->target->location.column == 2 &&
                assignment->target->location.offset == 5, "Wrong assignment target position");
        const auto* binary = dynamic_cast<Binary_Expr*>(assignment->expression.get());
        require(binary && binary->operation == Token_Type::Plus && binary->location.column == 9 &&
                binary->location.offset == 12, "Wrong binary position");
        const auto* unary = dynamic_cast<Unary_Expr*>(binary->left.get());
        require(unary && unary->location.column == 6 && unary->location.offset == 9,
                "Wrong unary position");
        const auto* integer = dynamic_cast<Integer_Expr*>(unary->operand.get());
        require(integer && integer->value == 2 && integer->location.column == 7 &&
                integer->location.offset == 10, "Wrong integer position");
        const auto* input = dynamic_cast<Input_Expr*>(binary->right.get());
        require(input && input->location.column == 11 && input->location.offset == 14,
                "Wrong input position");
        const auto* print = dynamic_cast<Print_Stmt*>(block->statements.at(1).get());
        require(print && print->location.line == 4 && print->location.column == 2,
                "Wrong print position");
        const auto* variable = dynamic_cast<Variable_Expr*>(print->expression.get());
        require(variable && variable->name == "x" && variable->location.column == 8,
                "Wrong variable position");

        // A conventional Fibonacci program; the original assignment is absent from the repository.
        const auto fibonacci = parse(path,
            "n=?; a=0; b=1; i=0; while(i<n){print a; next=a+b; a=b; b=next; i=i+1;}");
        require(fibonacci->statements.size() == 5, "Wrong Fibonacci program structure");
        const auto* loop = dynamic_cast<While_Stmt*>(fibonacci->statements.at(4).get());
        require(loop && loop->body->statements.size() == 5, "Wrong Fibonacci loop structure");
        const auto* condition = dynamic_cast<Binary_Expr*>(loop->condition.get());
        require(condition && condition->operation == Token_Type::Less, "Wrong Fibonacci condition");
        const auto* next = dynamic_cast<Assignment_Stmt*>(loop->body->statements.at(1).get());
        require(next && next->target && next->target->name == "next" && dynamic_cast<Binary_Expr*>(next->expression.get()),
                "Wrong Fibonacci update");

        expect_error(path, "x = ;", "1:5", "выражение", "';'");
        expect_error(path, "print", "1:6", "выражение", "конец файла");
        expect_error(path, "x=1", "1:4", "';'", "конец файла");
        expect_error(path, "print (1;", "1:9", "')'", "';'");
        expect_error(path, "{", "1:2", "'}'", "конец файла");
        expect_error(path, "if(1) print 2;", "1:7", "'{'", "'print'");
        expect_error(path, "while(1)", "1:9", "'{'", "конец файла");
        expect_error(path, "else {}", "1:1", "начало инструкции", "'else'");
        expect_error(path, "2+3;", "1:1", "начало инструкции", "'2'");
        expect_error(path, "a=b=1;", "1:4", "';'", "'='");
        expect_error(path, "if(1){}else print 2;", "1:13", "'{'", "'print'");
        expect_error(path, "if 1 {}", "1:4", "'('", "'1'");
        expect_error(path, "if(1 {}", "1:6", "')'", "'{'");
        expect_error(path, "print ();", "1:8", "выражение", "')'");
        expect_error(path, "print 1+;", "1:9", "выражение", "';'");
        expect_error(path, "print (", "1:8", "выражение", "конец файла");
        expect_error(path, "x 1;", "1:3", "'='", "'1'");
        expect_error(path, "}", "1:1", "начало инструкции", "'}'");
        expect_error(path, "\nprint 1\nx=2;", "3:1", "';'", "'x'");
        expect_error(path, "print 9223372036854775808;", "1:7", "целое число", "'9223372036854775808'");
        expect_error(path, "print 999999999999999999999999999999999;", "1:7", "целое число",
                     "'999999999999999999999999999999999'");

        std::cout << "Parser tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
