#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <memory>
#include <string>

#include "token.hpp"

namespace Lexer_namespace
{
class Lexer
{
    private:
        std::string code_text;
        std::string::iterator iterator_code = code_text.begin();

        Source_Location location;

        void advance();
        void skip_spaces_and_comments();
        std::string read_file(const std::string& file_name);

    public:
        std::unique_ptr<Token_Value> next_token();

        Lexer() = default;

        Lexer(const std::string& file_name): code_text(read_file(file_name)), iterator_code(code_text.begin())
        {
        }

        Lexer(const Lexer&) = delete;
        Lexer& operator=(const Lexer&) = delete;
        Lexer(Lexer&&) = delete;
        Lexer& operator=(Lexer&&) = delete;
};

}

#endif
