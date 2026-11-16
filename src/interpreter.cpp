#include "interpreter.hpp"

#include <iostream>
#include <limits>
#include <stdexcept>

Interpreter::Interpreter(): input(std::cin), output(std::cout)
{
}

void Interpreter::run_code(const Node* program)
{
    if (program -> node_type == Node_Type::Assignment)
    {
        const Assignment_Stmt* assignment = static_cast<const Assignment_Stmt*>(program);
        assign_variable((assignment -> target) -> name, evaluate((assignment -> expression).get()));
        return;
    }

    if (program -> node_type == Node_Type::Block)
    {
        const Block_Stmt* block = static_cast<const Block_Stmt*>(program);
        scopes.emplace_back();
        try
        {
            std::size_t size_stat = block -> statements.size();
            for (std::size_t i = 0; i < size_stat; i++)
            {
                Node* block_node = block -> statements.at(i).get();
                run_code(block_node);
            }
        }
        catch(...)
        {
            scopes.pop_back();
            throw;
        }
        scopes.pop_back();

        return;
    }

    if (program -> node_type == Node_Type::Print)
    {
        const Print_Stmt* print = static_cast<const Print_Stmt*>(program);
        output << evaluate((print -> expression).get()) << "\n";
        return;
    }
    
    if (program -> node_type == Node_Type::If)
    {
        const If_Stmt* conditional = static_cast<const If_Stmt*>(program);
        if (condition(conditional -> condition.get()))
        {
            run_code(conditional -> body.get());
        }
        else if (conditional -> alternative)
        {
            run_code(conditional -> alternative.get());
        }
        return;
    }
    
    if (program -> node_type == Node_Type::While)
    {
        const While_Stmt* loop = static_cast<const While_Stmt*>(program);
        
        while (condition(loop -> condition.get()))
        {
            run_code(loop -> body.get());
        }
        return;
    }
    return;
}

std::int64_t Interpreter::evaluate(const Expr* node)
{
    if (node -> node_type == Node_Type::Binary)
    {
        const Binary_Expr* binary = static_cast<const Binary_Expr*>(node);
        std::int64_t left = evaluate(binary -> left.get());
        std::int64_t right = evaluate(binary -> right.get());

        constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
        constexpr auto maximum = std::numeric_limits<std::int64_t>::max();

        switch (binary -> operation)
        {
            case Token_Type::Plus:
                if ((right > 0 && left > maximum - right) || (right < 0 && left < minimum - right))
                {
                    throw std::runtime_error("переполнение int64_t при сложении");
                }
                return left + right;
            case Token_Type::Minus:
                if ((right > 0 && left < minimum + right) || (right < 0 && left > maximum + right))
                {
                    throw std::runtime_error("переполнение int64_t при вычитании");
                }
                return left - right;
            case Token_Type::Multiplic:
                if ((left > 0 && right > 0 && left > maximum / right) ||
                    (left > 0 && right < 0 && right < minimum / left) ||
                    (left < 0 && right > 0 && left < minimum / right) ||
                    (left < 0 && right < 0 && left < maximum / right))
                {
                    throw std::runtime_error("переполнение int64_t при умножении");
                }
                return left * right;
            case Token_Type::Division:
            case Token_Type::Percent:
                if (right == 0)
                {
                    throw std::runtime_error("деление на ноль");
                }
                return (binary -> operation == Token_Type::Division) ? left / right : left % right;
            case Token_Type::Less: return left < right;
            case Token_Type::LessEqual: return left <= right;
            case Token_Type::Greater: return left > right;
            case Token_Type::GreaterEqual: return left >= right;
            case Token_Type::Equal: return left == right;
            case Token_Type::NotEqual: return left != right;
            default: throw std::runtime_error("несуществующая бинарная операция");
        }

    }
    if (node -> node_type == Node_Type::Unary)
    {
        const Unary_Expr* unar = static_cast<const Unary_Expr*>(node);
        std::int64_t left = evaluate(unar -> operand.get());

        switch (unar -> operation)
        {
            case Token_Type::Plus: return left;
            case Token_Type::Minus:
                if (left == std::numeric_limits<std::int64_t>::min())
                    throw std::runtime_error("переполнение int64_t при смене знака");
                return -left;
            default: throw std::runtime_error("несуществующая унарная операция");
        }
    }
    if (node -> node_type == Node_Type::Variable)
    {
        return read_variable(static_cast<const Variable_Expr*>(node) -> name); 
    }
    if (node -> node_type == Node_Type::Integer)
    {
        return static_cast<const Integer_Expr*>(node) -> value;
    }
    if (node -> node_type == Node_Type::Input)
    {
        std::int64_t value;
        if (!(input >> value))
        {
            throw std::runtime_error("ошибка ввода целого числа");
        }
        return value;
    }
    throw std::runtime_error("ожидалось выражение");
}

void Interpreter::assign_variable(const std::string& name, std::int64_t value)
{
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
    {
        auto variable = scope -> find(name);
        
        if (variable != scope -> end())
        {
            variable -> second = value;
            return;
        }
    }

    scopes.back().emplace(name, value);
}

std::int64_t Interpreter::read_variable(const std::string& name) const
{
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
    {
        const auto variable = scope -> find(name);

        if (variable != scope -> end())
        {
            return variable -> second;
        }
    }

    throw std::runtime_error("Переменная '" + name + "' не объявлена");
}

bool Interpreter::condition(const Expr* expression)
{
    return evaluate(expression) != 0;
}
