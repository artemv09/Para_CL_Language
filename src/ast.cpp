#include "ast.hpp"

#include "graphviz.hpp"
#include <utility>

Node::Node(Source_Location location, Token_Type token_type): location(location), token_type(token_type)
{
}

namespace
{
    const char* operator_name(Token_Type operation)
    {
        switch (operation)
        {
            case Token_Type::Plus: return "+";
            case Token_Type::Minus: return "-";
            case Token_Type::Multiplic: return "*";
            case Token_Type::Division: return "/";
            case Token_Type::Percent: return "%";
            case Token_Type::Less: return "<";
            case Token_Type::LessEqual: return "<=";
            case Token_Type::Greater: return ">";
            case Token_Type::GreaterEqual: return ">=";
            case Token_Type::Equal: return "==";
            case Token_Type::NotEqual: return "!=";
            default: return "unknown";
        }
    }
}

Integer_Expr::Integer_Expr(Source_Location location, std::int64_t value): Expr(location, Token_Type::Integer), value(value)
{
}

std::size_t Integer_Expr::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("Integer_Expr\n" + std::to_string(value));
    return id;
}

Variable_Expr::Variable_Expr(Source_Location location, std::string name): Expr(location, Token_Type::Identifier), name(std::move(name))
{
}

std::size_t Variable_Expr::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("Variable_Expr\n" + name);
    return id;
}

Input_Expr::Input_Expr(Source_Location location): Expr(location, Token_Type::Question)
{
}

std::size_t Input_Expr::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("Input_Expr\n?");
    return id;
}

Unary_Expr::Unary_Expr(Source_Location location, Token_Type operation, std::unique_ptr<Expr> operand): Expr(location, operation), operation(operation), operand(std::move(operand))
{
}

std::size_t Unary_Expr::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node(std::string("Unary_Expr\n") + operator_name(operation));
    
    writer.add_edge(id, operand -> write_graphviz(writer));
    return id;
}

Binary_Expr::Binary_Expr(Source_Location location, Token_Type operation, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right): Expr(location, operation), operation(operation), left(std::move(left)), right(std::move(right))
{
}

std::size_t Binary_Expr::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node(std::string("Binary_Expr\n") + operator_name(operation));

    writer.add_edge(id, left -> write_graphviz(writer));
    writer.add_edge(id, right -> write_graphviz(writer));
    return id;
}

Assignment_Stmt::Assignment_Stmt(Source_Location location, std::string name, std::unique_ptr<Expr> expression): Stmt(location, Token_Type::Assign), name(std::move(name)), expression(std::move(expression))
{
}

std::size_t Assignment_Stmt::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("Assignment_Stmt\n" + name + " =");

    writer.add_edge(id, expression -> write_graphviz(writer));
    return id;
}

Print_Stmt::Print_Stmt(Source_Location location, std::unique_ptr<Expr> expression): Stmt(location, Token_Type::Print), expression(std::move(expression))
{
}

std::size_t Print_Stmt::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("Print_Stmt\nprint");

    writer.add_edge(id, expression -> write_graphviz(writer));
    return id;
}

Block_Stmt::Block_Stmt(Source_Location location): Stmt(location, Token_Type::LeftBrace)
{
}

std::size_t Block_Stmt::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("Block_Stmt\n{}");

    for (const auto& statement : statements)
    {
        writer.add_edge(id, statement -> write_graphviz(writer));
    }
    return id;
}

If_Stmt::If_Stmt(Source_Location location, std::unique_ptr<Expr> condition, std::unique_ptr<Block_Stmt> body, std::unique_ptr<Stmt> alternative): Stmt(location, Token_Type::If), condition(std::move(condition)), body(std::move(body)), alternative(std::move(alternative))
{
}

std::size_t If_Stmt::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("If_Stmt\nif");

    writer.add_edge(id, condition -> write_graphviz(writer));
    writer.add_edge(id, body -> write_graphviz(writer));
    if (alternative)
    {
        writer.add_edge(id, alternative->write_graphviz(writer));
    }
    return id;
}

While_Stmt::While_Stmt(Source_Location location, std::unique_ptr<Expr> condition, std::unique_ptr<Block_Stmt> body): Stmt(location, Token_Type::While), condition(std::move(condition)), body(std::move(body))
{
}

std::size_t While_Stmt::write_graphviz(Graphviz_Writer& writer) const
{
    const auto id = writer.add_node("While_Stmt\nwhile");

    writer.add_edge(id, condition -> write_graphviz(writer));
    writer.add_edge(id, body -> write_graphviz(writer));
    return id;
}
