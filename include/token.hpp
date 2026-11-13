#ifndef TOKEN_HPP_
#define TOKEN_HPP_

#include <memory>
#include <string>

enum class Token_Type
{
    EndFile,
    Identifier,
    Integer,

    If,
    Else,
    While,
    Print,

    Plus,
    Minus,
    Multiplic,
    Division,
    Percent,

    Assign,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,
    Question
};

typedef struct 
{
    Token_Type token_type;
    std::string token_name;
} Token_Value;

std::unique_ptr<Token_Value> create_token(Token_Type token_type, std::string token_name);

#endif
