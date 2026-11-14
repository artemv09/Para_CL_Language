#ifndef AST_HPP_
#define AST_HPP_

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "token.hpp"

class Graphviz_Writer;

class Node
{
    public:
        Source_Location location;
        Token_Type token_type;

        explicit Node(Source_Location location, Token_Type token_type = Token_Type::EndFile);
        
        virtual ~Node() = default;

        virtual std::size_t write_graphviz(Graphviz_Writer& writer) const = 0;
};

class Expr : public Node
{
    public:
        using Node::Node;
};

class Stmt : public Node
{
    public:
        using Node::Node;
};

class Integer_Expr : public Expr
{
    public:
        std::int64_t value;

        explicit Integer_Expr(Source_Location location, std::int64_t value);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Variable_Expr : public Expr
{
    public:
        std::string name;

        explicit Variable_Expr(Source_Location location, std::string name);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Input_Expr : public Expr
{
    public:

        explicit Input_Expr(Source_Location location);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Unary_Expr : public Expr
{
    public:
        Token_Type operation;
        std::unique_ptr<Expr> operand;

        explicit Unary_Expr(Source_Location location, Token_Type operation, std::unique_ptr<Expr> operand);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Binary_Expr : public Expr
{
    public:
        Token_Type operation;
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;

        explicit Binary_Expr(Source_Location location, Token_Type operation, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Assignment_Stmt : public Stmt
{
    public:
        std::string name;
        std::unique_ptr<Expr> expression;

        explicit Assignment_Stmt(Source_Location location, std::string name, std::unique_ptr<Expr> expression);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Print_Stmt : public Stmt
{
    public:
        std::unique_ptr<Expr> expression;

        explicit Print_Stmt(Source_Location location, std::unique_ptr<Expr> expression);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class Block_Stmt : public Stmt
{
    public:
        std::vector<std::unique_ptr<Stmt>> statements;

        explicit Block_Stmt(Source_Location location);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class If_Stmt : public Stmt
{
    public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Block_Stmt> body;
        std::unique_ptr<Stmt> alternative;

        explicit If_Stmt(Source_Location location, std::unique_ptr<Expr> condition, std::unique_ptr<Block_Stmt> body, std::unique_ptr<Stmt> alternative);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

class While_Stmt : public Stmt
{
    public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<Block_Stmt> body;

        explicit While_Stmt(Source_Location location, std::unique_ptr<Expr> condition, std::unique_ptr<Block_Stmt> body);
        std::size_t write_graphviz(Graphviz_Writer& writer) const override;
};

#endif
