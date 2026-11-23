#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <sstream>

#include "graphviz.hpp"
#include "test_program.hpp"

namespace
{
    template<class T, class U>
    const T& node_as(const std::unique_ptr<U>& node)
    {
        if (!node)
        {
            throw std::logic_error("В дереве отсутствует ожидаемый узел");
        }
        return dynamic_cast<const T&>(*node);
    }

    void expect_location(const Node& node, std::size_t line, std::size_t column, std::size_t offset)
    {
        EXPECT_EQ(node.location.line, line);
        EXPECT_EQ(node.location.column, column);
        EXPECT_EQ(node.location.offset, offset);
    }

    void expect_parse_error(const std::string& source, const std::string& expected)
    {
        SCOPED_TRACE(source);
        try
        {
            parse_program(source);
            FAIL() << "Ожидалась ошибка разбора";
        }
        catch (const std::runtime_error& error)
        {
            const std::string message = error.what();
            EXPECT_NE(message.find(expected), std::string::npos) << message;
        }
    }
}

TEST(ParserTest, EmptyProgramProducesEmptyBlock)
{
    const auto program = parse_program("");

    ASSERT_NE(program, nullptr);
    EXPECT_TRUE(program -> statements.empty());
    EXPECT_EQ(program -> node_type, Node_Type::Block);
    expect_location(*program, 1, 1, 0);
}

TEST(ParserTest, ParsesAssignmentTargetAndExpression)
{
    const auto program = parse_program("x=52;");
    ASSERT_EQ(program -> statements.size(), 1u);
    const auto& assignment = node_as<Assignment_Stmt>(program -> statements.at(0));

    EXPECT_EQ(node_as<Variable_Expr>(assignment.target).name, "x");
    EXPECT_EQ(node_as<Integer_Expr>(assignment.expression).value, 52);
}

TEST(ParserTest, ParsesPrintAndInput)
{
    const auto program = parse_program("print ?;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    EXPECT_NE(dynamic_cast<const Input_Expr*>(print.expression.get()), nullptr);
}

TEST(ParserTest, PreservesStatementOrder)
{
    const auto program = parse_program("x=1; print x; x=2;");
    ASSERT_EQ(program -> statements.size(), 3);
    const auto& first = node_as<Assignment_Stmt>(program -> statements.at(0));
    const auto& print = node_as<Print_Stmt>(program -> statements.at(1));
    const auto& last = node_as<Assignment_Stmt>(program -> statements.at(2));

    EXPECT_EQ(node_as<Integer_Expr>(first.expression).value, 1);
    EXPECT_EQ(node_as<Variable_Expr>(print.expression).name, "x");
    EXPECT_EQ(node_as<Integer_Expr>(last.expression).value, 2);
}

TEST(ParserTest, MultiplicationBindsTighterThanAddition)
{
    const auto program = parse_program("print 2+3*4;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& nested = node_as<Binary_Expr>(root.right);

    EXPECT_EQ(root.operation, Token_Type::Plus);
    EXPECT_EQ(node_as<Integer_Expr>(root.left).value, 2);
    EXPECT_EQ(nested.operation, Token_Type::Multiplic);
    EXPECT_EQ(node_as<Integer_Expr>(nested.left).value, 3);
    EXPECT_EQ(node_as<Integer_Expr>(nested.right).value, 4);
}

TEST(ParserTest, ParenthesesOverridePrecedence)
{
    const auto program = parse_program("print ((2+3))*4;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& nested = node_as<Binary_Expr>(root.left);

    EXPECT_EQ(root.operation, Token_Type::Multiplic);
    EXPECT_EQ(node_as<Integer_Expr>(root.right).value, 4);
    EXPECT_EQ(nested.operation, Token_Type::Plus);
    EXPECT_EQ(node_as<Integer_Expr>(nested.left).value, 2);
    EXPECT_EQ(node_as<Integer_Expr>(nested.right).value, 3);
}

TEST(ParserTest, SubtractionIsLeftAssociative)
{
    const auto program = parse_program("print 10-3-2;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& nested = node_as<Binary_Expr>(root.left);

    EXPECT_EQ(root.operation, Token_Type::Minus);
    EXPECT_EQ(node_as<Integer_Expr>(root.right).value, 2);
    EXPECT_EQ(nested.operation, Token_Type::Minus);
    EXPECT_EQ(node_as<Integer_Expr>(nested.left).value, 10);
    EXPECT_EQ(node_as<Integer_Expr>(nested.right).value, 3);
}

TEST(ParserTest, DivisionAndModuloAreLeftAssociative)
{
    const auto program = parse_program("print 20/3%2;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& nested = node_as<Binary_Expr>(root.left);

    EXPECT_EQ(root.operation, Token_Type::Percent);
    EXPECT_EQ(node_as<Integer_Expr>(root.right).value, 2);
    EXPECT_EQ(nested.operation, Token_Type::Division);
    EXPECT_EQ(node_as<Integer_Expr>(nested.left).value, 20);
    EXPECT_EQ(node_as<Integer_Expr>(nested.right).value, 3);
}

TEST(ParserTest, ParsesNestedUnaryOperators)
{
    const auto program = parse_program("print -+x*2;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& product = node_as<Binary_Expr>(print.expression);
    const auto& minus = node_as<Unary_Expr>(product.left);
    const auto& plus = node_as<Unary_Expr>(minus.operand);

    EXPECT_EQ(product.operation, Token_Type::Multiplic);
    EXPECT_EQ(minus.operation, Token_Type::Minus);
    EXPECT_EQ(plus.operation, Token_Type::Plus);
    EXPECT_EQ(node_as<Variable_Expr>(plus.operand).name, "x");
}

TEST(ParserTest, LogicalOrContainsLogicalAndAndUnaryNot)
{
    const auto program = parse_program("print a || b && !c;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& conjunction = node_as<Binary_Expr>(root.right);
    const auto& negation = node_as<Unary_Expr>(conjunction.right);

    EXPECT_EQ(root.operation, Token_Type::LogicOr);
    EXPECT_EQ(node_as<Variable_Expr>(root.left).name, "a");
    EXPECT_EQ(conjunction.operation, Token_Type::LogicAnd);
    EXPECT_EQ(node_as<Variable_Expr>(conjunction.left).name, "b");
    EXPECT_EQ(negation.operation, Token_Type::LogicNo);
    EXPECT_EQ(node_as<Variable_Expr>(negation.operand).name, "c");
}

TEST(ParserTest, EqualityBindsTighterThanLogicalAnd)
{
    const auto program = parse_program("print a && b == c;");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& equality = node_as<Binary_Expr>(root.right);

    EXPECT_EQ(root.operation, Token_Type::LogicAnd);
    EXPECT_EQ(equality.operation, Token_Type::Equal);
    EXPECT_EQ(node_as<Variable_Expr>(equality.left).name, "b");
    EXPECT_EQ(node_as<Variable_Expr>(equality.right).name, "c");
}

TEST(ParserTest, ParenthesesOverrideLogicalPrecedence)
{
    const auto program = parse_program("print (a || b) && !(c == d);");
    const auto& print = node_as<Print_Stmt>(program -> statements.at(0));
    const auto& root = node_as<Binary_Expr>(print.expression);
    const auto& negation = node_as<Unary_Expr>(root.right);

    EXPECT_EQ(root.operation, Token_Type::LogicAnd);
    EXPECT_EQ(node_as<Binary_Expr>(root.left).operation, Token_Type::LogicOr);
    EXPECT_EQ(negation.operation, Token_Type::LogicNo);
    EXPECT_EQ(node_as<Binary_Expr>(negation.operand).operation, Token_Type::Equal);
}

TEST(ParserTest, LogicalOperatorsAreLeftAssociative)
{
    const auto program = parse_program("print a || b || c; print a && b && c;");
    const Token_Type operations[] = {Token_Type::LogicOr, Token_Type::LogicAnd};
    ASSERT_EQ(program -> statements.size(), std::size(operations));

    for (std::size_t i = 0; i < std::size(operations); ++i)
    {
        const auto& print = node_as<Print_Stmt>(program -> statements.at(i));
        const auto& root = node_as<Binary_Expr>(print.expression);
        const auto& nested = node_as<Binary_Expr>(root.left);

        EXPECT_EQ(root.operation, operations[i]);
        EXPECT_EQ(nested.operation, operations[i]);
        EXPECT_EQ(node_as<Variable_Expr>(nested.left).name, "a");
        EXPECT_EQ(node_as<Variable_Expr>(nested.right).name, "b");
        EXPECT_EQ(node_as<Variable_Expr>(root.right).name, "c");
    }
}

TEST(ParserTest, ParsesNestedBlocks)
{
    const auto program = parse_program("{} {{}}");
    ASSERT_EQ(program -> statements.size(), 2u);
    const auto& first = node_as<Block_Stmt>(program -> statements.at(0));
    const auto& second = node_as<Block_Stmt>(program -> statements.at(1));

    EXPECT_TRUE(first.statements.empty());
    ASSERT_EQ(second.statements.size(), 1u);
    EXPECT_TRUE(node_as<Block_Stmt>(second.statements.at(0)).statements.empty());
}

TEST(ParserTest, IfWithoutElseHasNoAlternative)
{
    const auto program = parse_program("if(x){}");
    const auto& branch = node_as<If_Stmt>(program -> statements.at(0));

    EXPECT_EQ(node_as<Variable_Expr>(branch.condition).name, "x");
    ASSERT_NE(branch.body, nullptr);
    EXPECT_TRUE(node_as<Block_Stmt>(branch.body).statements.empty());
    EXPECT_EQ(branch.alternative, nullptr);
}

TEST(ParserTest, ParsesElseIfChain)
{
    for (const std::string ending : {"", "else{}"})
    {
        SCOPED_TRACE(ending);
        const auto program = parse_program("if(a){}else if(b){}else if(c){}" + ending);
        const auto& first = node_as<If_Stmt>(program -> statements.at(0));
        const auto& second = node_as<If_Stmt>(first.alternative);
        const auto& third = node_as<If_Stmt>(second.alternative);

        EXPECT_EQ(node_as<Variable_Expr>(first.condition).name, "a");
        EXPECT_EQ(node_as<Variable_Expr>(second.condition).name, "b");
        EXPECT_EQ(node_as<Variable_Expr>(third.condition).name, "c");
        if (ending.empty())
        {
            EXPECT_EQ(third.alternative, nullptr);
        }
        else
        {
            EXPECT_TRUE(node_as<Block_Stmt>(third.alternative).statements.empty());
        }
    }
}

TEST(ParserTest, ParsesWhileConditionAndBody)
{
    const auto program = parse_program("while(x>0){x=x-1;}");
    const auto& loop = node_as<While_Stmt>(program -> statements.at(0));
    const auto& condition = node_as<Binary_Expr>(loop.condition);

    EXPECT_EQ(condition.operation, Token_Type::Greater);
    EXPECT_EQ(node_as<Variable_Expr>(condition.left).name, "x");
    EXPECT_EQ(node_as<Integer_Expr>(condition.right).value, 0);
    ASSERT_NE(loop.body, nullptr);
    ASSERT_EQ(loop.body -> statements.size(), 1u);
    const auto& assignment = node_as<Assignment_Stmt>(loop.body -> statements.at(0));
    EXPECT_EQ(node_as<Variable_Expr>(assignment.target).name, "x");
    const auto& difference = node_as<Binary_Expr>(assignment.expression);
    EXPECT_EQ(difference.operation, Token_Type::Minus);
    EXPECT_EQ(node_as<Variable_Expr>(difference.left).name, "x");
    EXPECT_EQ(node_as<Integer_Expr>(difference.right).value, 1);
}

TEST(ParserTest, PreservesNodeLocations)
{
    const auto program = parse_program("\n {\n  print x+12;\n }");
    const auto& block = node_as<Block_Stmt>(program -> statements.at(0));
    const auto& print = node_as<Print_Stmt>(block.statements.at(0));
    const auto& sum = node_as<Binary_Expr>(print.expression);

    expect_location(*program, 1, 1, 0);
    expect_location(block, 2, 2, 2);
    expect_location(print, 3, 3, 6);
    expect_location(node_as<Variable_Expr>(sum.left), 3, 9, 12);
    expect_location(sum, 3, 10, 13);
    expect_location(node_as<Integer_Expr>(sum.right), 3, 11, 14);
}

TEST(ParserTest, NodeTagsMatchDynamicTypes)
{
    const auto program = parse_program("x=?; if(x){while(x){print -x-1;}}");
    const auto& assignment = node_as<Assignment_Stmt>(program -> statements.at(0));
    const auto& branch = node_as<If_Stmt>(program -> statements.at(1));
    const auto& body = node_as<Block_Stmt>(branch.body);
    const auto& loop = node_as<While_Stmt>(body.statements.at(0));
    ASSERT_NE(loop.body, nullptr);
    const auto& print = node_as<Print_Stmt>(loop.body -> statements.at(0));
    const auto& difference = node_as<Binary_Expr>(print.expression);

    EXPECT_EQ(program -> node_type, Node_Type::Block);
    EXPECT_EQ(assignment.node_type, Node_Type::Assignment);
    EXPECT_EQ(node_as<Input_Expr>(assignment.expression).node_type, Node_Type::Input);
    EXPECT_EQ(node_as<Variable_Expr>(branch.condition).node_type, Node_Type::Variable);
    EXPECT_EQ(branch.node_type, Node_Type::If);
    EXPECT_EQ(body.node_type, Node_Type::Block);
    EXPECT_EQ(loop.node_type, Node_Type::While);
    EXPECT_EQ(print.node_type, Node_Type::Print);
    EXPECT_EQ(difference.node_type, Node_Type::Binary);
    EXPECT_EQ(node_as<Unary_Expr>(difference.left).node_type, Node_Type::Unary);
    EXPECT_EQ(node_as<Integer_Expr>(difference.right).node_type, Node_Type::Integer);
}

TEST(ParserTest, RejectsOutOfRangeInteger)
{
    const std::string literals[] = {"9223372036854775808", "-9223372036854775809", std::string(100, '9')};
    for (const auto& literal : literals)
    {
        expect_parse_error("print " + literal + ";", "диапазоне int64_t");
    }
}

TEST(ParserTest, RejectsMissingSemicolon)
{
    expect_parse_error("x=1", "';'");
    expect_parse_error("print 1", "';'");
    expect_parse_error("x=1\nprint x;", test_file_name() + ":2:1: ошибка: ожидалось ';', получен 'print'");
    expect_parse_error("if(1) print 2 else print 3;", "';', получен 'else'");
    expect_parse_error("if(1){} else x=1", "';', получен конец файла");
}

TEST(ParserTest, RejectsMissingOperand)
{
    for (const auto* source : {"x=;", "print 1+;", "print ();"})
    {
        expect_parse_error(source, "выражение");
    }
    expect_parse_error("print !;", test_file_name() + ":1:8: ошибка: ожидалось выражение, получен ';'");
    expect_parse_error("print 1 && ;", test_file_name() + ":1:12: ошибка: ожидалось выражение, получен ';'");
}

TEST(ParserTest, RejectsUnclosedDelimiters)
{
    expect_parse_error("print (1", "')', получен конец файла");
    expect_parse_error("{x=1;", "'}', получен конец файла");
}

TEST(ParserTest, RejectsInvalidStatementForms)
{
    expect_parse_error("else {}", test_file_name() + ":1:1: ошибка: 'else' без соответствующего 'if'");
    expect_parse_error("print (a || b = 1);", "имя переменной слева от '='");
    for (const auto* source : {"a=b=1;", "x+1;", "}"})
    {
        SCOPED_TRACE(source);
        EXPECT_THROW(parse_program(source), std::runtime_error);
    }
}

TEST(ParserTest, AcceptsEmptyStatement)
{
    const auto program = parse_program(";");
    ASSERT_NE(program, nullptr);
    EXPECT_EQ(program -> node_type, Node_Type::Block);
    EXPECT_TRUE(program -> statements.empty());
}

TEST(ParserTest, ParsesSingleStatementIfAndElseBodies)
{
    const auto program = parse_program("if(1) print 2; else x=3;");
    const auto& branch = node_as<If_Stmt>(program -> statements.at(0));
    const auto& print = node_as<Print_Stmt>(branch.body);
    const auto& assignment = node_as<Assignment_Stmt>(branch.alternative);

    EXPECT_EQ(node_as<Integer_Expr>(print.expression).value, 2);
    EXPECT_EQ(node_as<Variable_Expr>(assignment.target).name, "x");
    EXPECT_EQ(node_as<Integer_Expr>(assignment.expression).value, 3);
}

TEST(ParserTest, RequiresBracedWhileBody)
{
    expect_parse_error("while(1) print 2;", "'{', получен 'print'");
}
