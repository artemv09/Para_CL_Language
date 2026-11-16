#ifndef PARSER_HPP_
#define PARSER_HPP_

#include "ast.hpp"
#include "lexer.hpp"

class Parser
{
    private:
        Lexer_namespace::Lexer& lexer;
        std::unique_ptr<Token_Value> current_token;
        std::string file_name;

        void advance();
        bool check(Token_Type type) const;
        bool match(Token_Type type);
        Token_Value expect(Token_Type type, const std::string& expected);
        [[noreturn]] void error(const std::string& expected) const;

        std::unique_ptr<Stmt> parse_statement();
        std::unique_ptr<Block_Stmt> parse_block();
        std::unique_ptr<Stmt> parse_assignment();
        std::unique_ptr<Stmt> parse_print();
        std::unique_ptr<Stmt> parse_if();
        std::unique_ptr<Stmt> parse_while();
        std::unique_ptr<Expr> parse_expression();
        std::unique_ptr<Expr> parse_equality();
        std::unique_ptr<Expr> parse_comparison();
        std::unique_ptr<Expr> parse_additive();
        std::unique_ptr<Expr> parse_multiplicative();
        std::unique_ptr<Expr> parse_unary();
        std::unique_ptr<Expr> parse_primary();

    public:
        explicit Parser(Lexer_namespace::Lexer& lexer, std::string file_name);

        Parser(const Parser&) = delete;
        Parser& operator=(const Parser&) = delete;
        Parser(Parser&&) = delete;
        Parser& operator=(Parser&&) = delete;

        std::unique_ptr<Block_Stmt> parse_program();
};

#endif
