#include "semantic_analyzer.hpp"

void Segmantic_Analiz::segmantic_check(Node* node, Assigment_Parent assigment_referaral)
{
    if(node -> node_type == Node_Type::Variable)
    {
        const std::string& name = static_cast<Variable_Expr*>(node) -> name;

        if(assigment_referaral == Assigment_Parent::Right || assigment_referaral == Assigment_Parent::Non)
        {
            if(!checking_presence_advance(name))
            {
                error_undeclared(name, node -> location);
            }

            return;
        }
        else
        {
            if(!checking_presence_advance(name))
            {
                scopes.back().insert(name);
            }
            return;
        }
    }

    if(node -> node_type == Node_Type::Assignment)
    {
        Assignment_Stmt* assignment = static_cast<Assignment_Stmt*>(node);
        segmantic_check(assignment -> expression.get(), Assigment_Parent::Right);
        segmantic_check(assignment -> target.get(), Assigment_Parent::Left);
        return;
    }

    if(node -> node_type == Node_Type::Block)
    {
        Block_Stmt* block = static_cast<Block_Stmt*>(node);
        scopes.emplace_back();
        try
        {
            std::size_t size_stat = block -> statements.size();
            for(std::size_t i = 0; i < size_stat; i++)
            {
                Node* block_node = block -> statements.at(i).get();
                segmantic_check(block_node, Assigment_Parent::Non);
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

    if(node -> node_type == Node_Type::Unary)
    {
        Unary_Expr* unary = static_cast<Unary_Expr*>(node);
        segmantic_check(unary -> operand.get(), Assigment_Parent::Non);
        return;
    }
    
    if(node -> node_type == Node_Type::Binary)
    {
        Binary_Expr* binary = static_cast<Binary_Expr*>(node);
        segmantic_check(binary -> left.get(), Assigment_Parent::Non);
        segmantic_check(binary -> right.get(), Assigment_Parent::Non);
        return;
    }
    
    if(node -> node_type == Node_Type::Print)
    {
        Print_Stmt* print = static_cast<Print_Stmt*>(node);
        segmantic_check(print -> expression.get(), Assigment_Parent::Non);
        return;
    }
    
    if(node -> node_type == Node_Type::If)
    {
        If_Stmt* conditional = static_cast<If_Stmt*>(node);
        segmantic_check(conditional -> condition.get(), Assigment_Parent::Non);
        segmantic_check(conditional -> body.get(), Assigment_Parent::Non);
        if(conditional -> alternative)
        {
            segmantic_check(conditional -> alternative.get(), Assigment_Parent::Non);
        }
        return;
    }
    
    if(node -> node_type == Node_Type::While)
    {
        While_Stmt* loop = static_cast<While_Stmt*>(node);
        segmantic_check(loop -> condition.get(), Assigment_Parent::Non);
        segmantic_check(loop -> body.get(), Assigment_Parent::Non);
        return;
    }
    return;
}

bool Segmantic_Analiz::checking_presence_advance(const std::string& name) const
{
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
    {
        if (scope -> find(name) != scope -> end())
        {
            return true;
        }
    }

    return false;
}

void Segmantic_Analiz::error_undeclared(const std::string& name, const Source_Location& location) const
{
    const Diagnostic diagnostic{location, "переменная '" + name + "' не объявлена"};

    throw std::runtime_error(diagnostic.format(file_name));
}
