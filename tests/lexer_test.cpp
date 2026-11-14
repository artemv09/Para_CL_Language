#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using Lexer_namespace::Lexer;
using Type = Token_Type;

void require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void write_source(const std::string& path, const std::string& source)
{
    std::ofstream file(path, std::ios::binary);
    file << source;
    require(static_cast<bool>(file), "Cannot write test input");
}

void expect_token(Lexer& lexer, const Token_Value& expected)
{
    const auto token = lexer.next_token();
    require(token != nullptr, "next_token returned nullptr");
    require(token->token_name == expected.token_name,
            "Wrong token text; expected: " + expected.token_name);
    require(token->token_type == expected.token_type,
            "Wrong token type for: " + expected.token_name);
}

void expect_tokens(const std::string& path, const std::string& source,
                   const std::vector<Token_Value>& expected)
{
    write_source(path, source);
    Lexer lexer(path);
    for (const auto& token : expected)
    {
        expect_token(lexer, token);
    }
    expect_token(lexer, {Type::EndFile, ""});
    expect_token(lexer, {Type::EndFile, ""});
}

void expect_locations(const std::string& path, const std::string& source,
                      const std::vector<Token_Value>& expected)
{
    write_source(path, source);
    Lexer lexer(path);
    for (const auto& token : expected)
    {
        const auto actual = lexer.next_token();
        require(actual->token_type == token.token_type && actual->token_name == token.token_name,
                "Wrong token while checking location");
        require(actual->location.line == token.location.line &&
                actual->location.column == token.location.column &&
                actual->location.offset == token.location.offset,
                "Wrong location for: " + token.token_name);
    }
}

void expect_error(const std::string& path, const std::string& source)
{
    write_source(path, source);
    Lexer lexer(path);
    try
    {
        while (lexer.next_token()->token_type != Type::EndFile)
        {
        }
    }
    catch (const std::runtime_error&)
    {
        return;
    }
    throw std::runtime_error("Invalid input was accepted: " + source);
}

int main(int argc, char* argv[])
{
    try
    {
        require(argc == 2, "Pass a temporary input file path");
        const std::string path = argv[1];

        expect_locations(path, "name=12!=3;",
                         {{Type::Identifier, "name", {1, 1, 0}},
                          {Type::Assign, "=", {1, 5, 4}},
                          {Type::Integer, "12", {1, 6, 5}},
                          {Type::NotEqual, "!=", {1, 8, 7}},
                          {Type::Integer, "3", {1, 10, 9}},
                          {Type::Semicolon, ";", {1, 11, 10}},
                          {Type::EndFile, "", {1, 12, 11}},
                          {Type::EndFile, "", {1, 12, 11}}});
        expect_locations(path, "x\nprint\n",
                         {{Type::Identifier, "x", {1, 1, 0}},
                          {Type::Print, "print", {2, 1, 2}},
                          {Type::EndFile, "", {3, 1, 8}},
                          {Type::EndFile, "", {3, 1, 8}}});
        expect_locations(path, " \t// hi\n\t x//end",
                         {{Type::Identifier, "x", {2, 3, 10}},
                          {Type::EndFile, "", {2, 9, 16}},
                          {Type::EndFile, "", {2, 9, 16}}});
        expect_locations(path, "",
                         {{Type::EndFile, "", {1, 1, 0}},
                          {Type::EndFile, "", {1, 1, 0}}});
        expect_locations(path, " \t\n//end",
                         {{Type::EndFile, "", {2, 6, 8}},
                          {Type::EndFile, "", {2, 6, 8}}});

        expect_tokens(path, "", {});
        expect_tokens(path, " \t\r\n\v\f// comment", {});
        expect_tokens(path, "// first\n // second\nname_2=123;// end",
                      {{Type::Identifier, "name_2"}, {Type::Assign, "="},
                       {Type::Integer, "123"}, {Type::Semicolon, ";"}});
        expect_tokens(path, "if else while print ifelse Else while2 print_ _if",
                      {{Type::If, "if"}, {Type::Else, "else"}, {Type::While, "while"},
                       {Type::Print, "print"}, {Type::Identifier, "ifelse"},
                       {Type::Identifier, "Else"}, {Type::Identifier, "while2"},
                       {Type::Identifier, "print_"}, {Type::Identifier, "_if"}});
        expect_tokens(path, "_name A1 0 123", {{Type::Identifier, "_name"},
                      {Type::Identifier, "A1"}, {Type::Integer, "0"}, {Type::Integer, "123"}});

        const std::vector<Token_Value> operators = {
            {Type::Plus, "+"}, {Type::Minus, "-"}, {Type::Multiplic, "*"},
            {Type::Division, "/"}, {Type::Percent, "%"}, {Type::Assign, "="},
            {Type::Equal, "=="}, {Type::NotEqual, "!="}, {Type::Less, "<"},
            {Type::LessEqual, "<="}, {Type::Greater, ">"}, {Type::GreaterEqual, ">="},
            {Type::LeftParen, "("}, {Type::RightParen, ")"}, {Type::LeftBrace, "{"},
            {Type::RightBrace, "}"}, {Type::Semicolon, ";"}, {Type::Question, "?"}
        };
        expect_tokens(path, "+ - * / % = == != < <= > >= ( ) { } ; ?", operators);
        for (const auto& token : operators)
        {
            expect_tokens(path, token.token_name, {token});
        }

        expect_tokens(path, "if(x<=10){print x+2*3;}else{while(x!=0){x=x-1;}}",
                      {{Type::If, "if"}, {Type::LeftParen, "("}, {Type::Identifier, "x"},
                       {Type::LessEqual, "<="}, {Type::Integer, "10"}, {Type::RightParen, ")"},
                       {Type::LeftBrace, "{"}, {Type::Print, "print"}, {Type::Identifier, "x"},
                       {Type::Plus, "+"}, {Type::Integer, "2"}, {Type::Multiplic, "*"},
                       {Type::Integer, "3"}, {Type::Semicolon, ";"}, {Type::RightBrace, "}"},
                       {Type::Else, "else"}, {Type::LeftBrace, "{"}, {Type::While, "while"},
                       {Type::LeftParen, "("}, {Type::Identifier, "x"}, {Type::NotEqual, "!="},
                       {Type::Integer, "0"}, {Type::RightParen, ")"}, {Type::LeftBrace, "{"},
                       {Type::Identifier, "x"}, {Type::Assign, "="}, {Type::Identifier, "x"},
                       {Type::Minus, "-"}, {Type::Integer, "1"}, {Type::Semicolon, ";"},
                       {Type::RightBrace, "}"}, {Type::RightBrace, "}"}});
        expect_tokens(path, "a===b!==c", {{Type::Identifier, "a"}, {Type::Equal, "=="},
                      {Type::Assign, "="}, {Type::Identifier, "b"}, {Type::NotEqual, "!="},
                      {Type::Assign, "="}, {Type::Identifier, "c"}});
        expect_tokens(path, "x// comment\r\n/", {{Type::Identifier, "x"}, {Type::Division, "/"}});
        expect_tokens(path, " \tx=-12;", {{Type::Identifier, "x"}, {Type::Assign, "="},
                      {Type::Minus, "-"}, {Type::Integer, "12"}, {Type::Semicolon, ";"}});

        write_source(path, "first second");
        Lexer lexer(path);
        const auto first = lexer.next_token();
        expect_token(lexer, {Type::Identifier, "second"});
        expect_token(lexer, {Type::EndFile, ""});
        require(first->token_type == Type::Identifier && first->token_name == "first",
                "Reading another token changed the previous token");

        Lexer empty;
        expect_token(empty, {Type::EndFile, ""});
        expect_token(empty, {Type::EndFile, ""});
        for (const std::string source : {"@", "!", "!x", "&", "|", ".", "123abc", "12_name", "1.5", "\"text\""})
        {
            expect_error(path, source);
        }
        expect_error(path, std::string(1, '\0'));
        expect_error(path, std::string(1, static_cast<char>(0xff)));

        bool missing_file_reported = false;
        try
        {
            Lexer missing(path + ".missing");
        }
        catch (const std::runtime_error&)
        {
            missing_file_reported = true;
        }
        require(missing_file_reported, "Missing input file was not reported");

        std::cout << "Lexer tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
