#include "lexer.hpp"

#include <cctype>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace Lexer_namespace
{
    std::string Lexer::read_file(const std::string& file_name)
    {
        std::ifstream file(file_name);

        if (!file)
        {
            throw std::runtime_error("Cannot open file: " + file_name);
        }

        std::string text{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

        return text;
    }

    void Lexer::advance()
    {
        if (iterator_code == code_text.end())
        {
            return;
        }

        if (*iterator_code == '\n')
        {
            ++location.line;
            location.column = 1;
        }
        else
        {
            ++location.column;
        }

        ++location.offset;
        ++iterator_code;
    }

    void Lexer::skip_spaces_and_comments()
    {
        const auto end = code_text.end();

        while (true)
        {
            while (iterator_code != end && std::isspace(static_cast<unsigned char>(*iterator_code)))
            {
                advance();
            }

            if (iterator_code == end)
            {
                return;
            }

            if (*iterator_code == '/' && std::next(iterator_code) != end && *std::next(iterator_code) == '/')
            {
                advance();
                advance();

                while (iterator_code != end && *iterator_code != '\n')
                {
                    advance();
                }
                continue;
            }

            return;
        }
    }

    std::unique_ptr<Token_Value> Lexer::next_token()
    {
        skip_spaces_and_comments();

        const auto end = code_text.end();
        const auto token_location = location;
        
        if (iterator_code == end)
        {
            return create_token(Token_Type::EndFile, "", token_location);
        }

        const auto start = iterator_code;

        const auto is_identifier_start = [](char character)
        {
            return (character >= 'a' && character <= 'z') ||
                   (character >= 'A' && character <= 'Z') ||
                   (character == '_');
        };

        const auto is_digit = [](char character)
        {
            return character >= '0' && character <= '9';
        };

        if (is_identifier_start(*iterator_code))
        {
            do
            {
                advance();
            }
            while (iterator_code != end && (is_identifier_start(*iterator_code) || is_digit(*iterator_code)));

            std::string token_name(start, iterator_code);
            Token_Type token_type = Token_Type::Identifier;

            if (token_name == "if")
            {
                token_type = Token_Type::If;
            }
            else if (token_name == "else")
            {
                token_type = Token_Type::Else;
            }
            else if (token_name == "while")
            {
                token_type = Token_Type::While;
            }
            else if (token_name == "print")
            {
                token_type = Token_Type::Print;
            }

            return create_token(token_type, std::move(token_name), token_location);
        }

        if (is_digit(*iterator_code))
        {
            do
            {
                advance();
            }
            while (iterator_code != end && is_digit(*iterator_code));

            if (iterator_code != end && is_identifier_start(*iterator_code))
            {
                throw std::runtime_error("Invalid integer literal");
            }

            return create_token(Token_Type::Integer, std::string(start, iterator_code), token_location);
        }

        const char character = *iterator_code;
        advance();
        
        Token_Type token_type;

        switch (character)
        {
            case '=':
                if (iterator_code != end && *iterator_code == '=')
                {
                    advance();
                    token_type = Token_Type::Equal;
                }
                else
                {
                    token_type = Token_Type::Assign;
                }
                break;

            case '<':
                if (iterator_code != end && *iterator_code == '=')
                {
                    advance();
                    token_type = Token_Type::LessEqual;
                }
                else
                {
                    token_type = Token_Type::Less;
                }
                break;

            case '>':
                if (iterator_code != end && *iterator_code == '=')
                {
                    advance();
                    token_type = Token_Type::GreaterEqual;
                }
                else
                {
                    token_type = Token_Type::Greater;
                }
                break;

            case '!':
                if (iterator_code == end || *iterator_code != '=')
                {
                    throw std::runtime_error("Expected '=' after '!'");
                }
                advance();
                token_type = Token_Type::NotEqual;
                break;

            case '+':
                token_type = Token_Type::Plus;
                break;

            case '-':
                token_type = Token_Type::Minus;
                break;

            case '*':
                token_type = Token_Type::Multiplic;
                break;

            case '/':
                token_type = Token_Type::Division;
                break;

            case '%':
                token_type = Token_Type::Percent;
                break;

            case '(':
                token_type = Token_Type::LeftParen;
                break;

            case ')':
                token_type = Token_Type::RightParen;
                break;

            case '{':
                token_type = Token_Type::LeftBrace;
                break;

            case '}':
                token_type = Token_Type::RightBrace;
                break;

            case ';':
                token_type = Token_Type::Semicolon;
                break;

            case '?':
                token_type = Token_Type::Question;
                break;

            default:
                throw std::runtime_error(std::string("Unexpected character: ") + character);
        }

        return create_token(token_type, std::string(start, iterator_code), token_location);
    }

} //namespace Lexer
