#include "parser.hpp"

#include <charconv>
#include <stdexcept>
#include <system_error>
#include <utility>

Parser::Parser(Lexer& lexer, std::string file_name): lexer(lexer), file_name(std::move(file_name))
{
    advance();
}

void Parser::advance()
{
    current_token = lexer.next_token();
}

bool Parser::check(Token_Type type) const
{
    return current_token -> token_type == type;
}

bool Parser::match(Token_Type type)
{
    if (!check(type))
    {
        return false;
    }
    advance();
    return true;
}

Token_Value Parser::expect(Token_Type type, const std::string& expected)
{
    if (!check(type))
    {
        error(expected);
    }

    const auto token = *current_token;
    advance();

    return token;
}

void Parser::error(const std::string& expected) const
{
    const std::string received = check(Token_Type::EndFile) ? "конец файла" : "'" + current_token -> token_name + "'";
    const Diagnostic diagnostic{current_token -> location, "ожидалось " + expected + ", получен " + received};

    throw std::runtime_error(diagnostic.format(file_name));
}

std::unique_ptr<Block_Stmt> Parser::parse_program()
{
    auto program = std::make_unique<Block_Stmt>(Source_Location{});

    while (!check(Token_Type::EndFile))
    {
        if (match(Token_Type::Semicolon))
        {
            continue;
        }

        program -> statements.push_back(parse_statement());
    }
    return program;
}

std::unique_ptr<Stmt> Parser::parse_statement()
{
    switch (current_token -> token_type)
    {
        case Token_Type::Identifier: return parse_assignment();
        case Token_Type::Print: return parse_print();
        case Token_Type::LeftBrace: return parse_block();
        case Token_Type::If: return parse_if();
        case Token_Type::While: return parse_while();
        case Token_Type::Else:
        {
            const Diagnostic diagnostic{current_token -> location, "'else' без соответствующего 'if'"};
            throw std::runtime_error(diagnostic.format(file_name));
        }
        default: error("начало инструкции (имя переменной, 'print', '{', 'if' или 'while')");
    }
}

std::unique_ptr<Block_Stmt> Parser::parse_block()
{
    const auto token = expect(Token_Type::LeftBrace, "'{'");

    auto block = std::make_unique<Block_Stmt>(token.location);

    while (!check(Token_Type::RightBrace))
    {
        if (check(Token_Type::EndFile))
        {
            error("'}'");
        }
        block -> statements.push_back(parse_statement());
    }

    expect(Token_Type::RightBrace, "'}'");
    return block;
}

std::unique_ptr<Stmt> Parser::parse_assignment()
{
    const auto name = expect(Token_Type::Identifier, "имя переменной");

    auto target = std::make_unique<Variable_Expr>(name.location, name.token_name);

    expect(Token_Type::Assign, "'='");

    auto expression = parse_expression();

    expect(Token_Type::Semicolon, "';'");

    return std::make_unique<Assignment_Stmt>(name.location, std::move(target), std::move(expression));
}

std::unique_ptr<Stmt> Parser::parse_print()
{
    const auto token = expect(Token_Type::Print, "'print'");

    auto expression = parse_expression();

    expect(Token_Type::Semicolon, "';'");

    return std::make_unique<Print_Stmt>(token.location, std::move(expression));
}

std::unique_ptr<Stmt> Parser::parse_if()
{
    const auto token = expect(Token_Type::If, "'if'");

    expect(Token_Type::LeftParen, "'('");

    auto condition = parse_expression();

    expect(Token_Type::RightParen, "')'");

    auto body = parse_statement(); // change parse_block --> parse_statement

    std::unique_ptr<Stmt> alternative;

    if (match(Token_Type::Else))
    {
        alternative = parse_statement();
    }
    return std::make_unique<If_Stmt>(token.location, std::move(condition), std::move(body), std::move(alternative));
}

std::unique_ptr<Stmt> Parser::parse_while()
{
    const auto token = expect(Token_Type::While, "'while'");

    expect(Token_Type::LeftParen, "'('");

    auto condition = parse_expression();

    expect(Token_Type::RightParen, "')'");

    auto body = parse_block();

    return std::make_unique<While_Stmt>(token.location, std::move(condition), std::move(body));
}

std::unique_ptr<Expr> Parser::parse_expression()
{
    return parse_logical_or();
}

std::unique_ptr<Expr> Parser::parse_logical_or()
{
    auto left = parse_logical_and();

    while (check(Token_Type::LogicOr))
    {
        const auto operation = *current_token;
        advance();

        auto right = parse_logical_and();
        left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_logical_and()
{
    auto left = parse_equality();

    while (check(Token_Type::LogicAnd))
    {
        const auto operation = *current_token;
        advance();

        auto right = parse_equality();
        left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_equality()
{
    auto left = parse_comparison();

    while (check(Token_Type::Equal) || check(Token_Type::NotEqual))
    {
        const auto operation = *current_token;

        advance();

        auto right = parse_comparison();

        left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_comparison()
{
    auto left = parse_additive();
    
    while (check(Token_Type::Less) || check(Token_Type::LessEqual) || check(Token_Type::Greater) || check(Token_Type::GreaterEqual))
    {
        const auto operation = *current_token;

        advance();

        auto right = parse_additive();

        left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_additive()
{
    auto left = parse_multiplicative();

    while (check(Token_Type::Plus) || check(Token_Type::Minus))
    {
        const auto operation = *current_token;

        advance();

        auto right = parse_multiplicative();

        left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_multiplicative()
{
    auto left = parse_unary();
    while (check(Token_Type::Multiplic) || check(Token_Type::Division) || check(Token_Type::Percent))
    {
        const auto operation = *current_token;

        advance();

        auto right = parse_unary();

        left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_unary()
{
    if (check(Token_Type::Plus) || check(Token_Type::Minus) || check(Token_Type::LogicNo))
    {
        const auto operation = *current_token;

        advance();

        auto operand = parse_unary();

        return std::make_unique<Unary_Expr>(operation.location, operation.token_type, std::move(operand));
    }
    return parse_primary();
}

std::unique_ptr<Expr> Parser::parse_primary()
{
    const auto token = *current_token;

    if (check(Token_Type::Integer))
    {
        std::int64_t value = 0;

        const auto begin = token.token_name.data();
        const auto end = begin + token.token_name.size();
        const auto result = std::from_chars(begin, end, value);

        if (result.ec != std::errc{} || result.ptr != end)
        {
            error("целое число в диапазоне int64_t");
        }

        advance();
        return std::make_unique<Integer_Expr>(token.location, value);
    }
    if (match(Token_Type::Identifier))
    {
        return std::make_unique<Variable_Expr>(token.location, token.token_name);
    }
    if (match(Token_Type::Question))
    {
        return std::make_unique<Input_Expr>(token.location);
    }
    if (match(Token_Type::LeftParen))
    {
        auto left = parse_expression();

        if (check(Token_Type::Assign))
        {
            if (left -> node_type != Node_Type::Variable)
            {
                error("имя переменной слева от '='");
            }

            const auto operation = *current_token;
            advance();                       
            auto right = parse_expression(); 

            left = std::make_unique<Binary_Expr>(operation.location, operation.token_type, std::move(left), std::move(right));
        }

        expect(Token_Type::RightParen, "')'");
        return left;
    }
    error("выражение");
}
