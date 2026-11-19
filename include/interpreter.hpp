#ifndef INTERPETER_HPP_
#define INTERPETER_HPP_

#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

class Interpreter
{
    private:
        std::vector<std::unordered_map<std::string, std::int64_t>> scopes;

        std::istream& input;
        std::ostream& output;

        void assign_variable(const std::string& name, std::int64_t value);
        std::int64_t read_variable(const std::string& name) const;
        bool condition(const Expr* expression);
        std::int64_t evaluate(const Expr* node);

    public:
        Interpreter();

        Interpreter(const Interpreter&) = delete;
        Interpreter& operator=(const Interpreter&) = delete;
        Interpreter(Interpreter&&) = delete;
        Interpreter& operator=(Interpreter&&) = delete;

        void run_code(const Node* program);
};

#endif
