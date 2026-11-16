#ifndef SEMANTIC_ANALYZER_HPP_
#define SEMANTIC_ANALYZER_HPP_

#include "parser.hpp"
#include "ast.hpp"

#include <vector>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <utility>

enum class Assigment_Parent
        {
            Right,
            Left,
            Non
        };

class Segmantic_Analiz
{
    private:
        using Root_Ast = std::unique_ptr<Block_Stmt>; 

        std::string file_name;
        std::vector<std::unordered_set<std::string>> scopes;

        bool checking_presence_advance(const std::string& name) const;
        void error_undeclared(const std::string& name, const Source_Location& location) const;

    public:
        Segmantic_Analiz(std::string file_name): file_name(std::move(file_name))
        {
        }

        void segmantic_check(Node* node, Assigment_Parent assigment_referaral = Assigment_Parent::Non);

    };

#endif
