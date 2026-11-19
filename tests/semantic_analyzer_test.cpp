#include <gtest/gtest.h>

#include "semantic_analyzer.hpp"
#include "test_program.hpp"

static void expect_undeclared(Segmantic_Analiz& analyzer, const std::string& source, const std::string& name)
{
    SCOPED_TRACE(source);
    const auto program = parse_program(source);
    try
    {
        analyzer.segmantic_check(program.get());
        FAIL() << "Ожидалась ошибка для переменной " << name;
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();
        EXPECT_NE(message.find("'" + name + "' не объявлена"), std::string::npos) << message;
    }
}

TEST(SemanticAnalyzerTest, AcceptsDeclaredVariable)
{
    Segmantic_Analiz analyzer(test_file_name());
    const auto program = parse_program("x=1; print x;");
    EXPECT_NO_THROW(analyzer.segmantic_check(program.get()));
}

TEST(SemanticAnalyzerTest, RejectsUndeclaredVariable)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "print x;", "x");
    expect_undeclared(analyzer, "a=b+1;", "b");
}

TEST(SemanticAnalyzerTest, ChecksInitializerBeforeDeclaration)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "x=x+1;", "x");
}

TEST(SemanticAnalyzerTest, AcceptsReassignment)
{
    Segmantic_Analiz analyzer(test_file_name());
    const auto program = parse_program("x=1; x=x+1;");
    EXPECT_NO_THROW(analyzer.segmantic_check(program.get()));
}

TEST(SemanticAnalyzerTest, FindsVariableInOuterScope)
{
    Segmantic_Analiz analyzer(test_file_name());
    const auto program = parse_program("x=1; {{print x; x=x+1;}}");
    EXPECT_NO_THROW(analyzer.segmantic_check(program.get()));
}

TEST(SemanticAnalyzerTest, RejectsVariableOutsideItsBlock)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "{x=1;} print x;", "x");
}

TEST(SemanticAnalyzerTest, DoesNotShareVariablesBetweenSiblingBlocks)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "{x=1;} {print x;}", "x");
}

TEST(SemanticAnalyzerTest, DoesNotShareVariablesBetweenBranches)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "if(1){x=1;}else{print x;}", "x");
}

TEST(SemanticAnalyzerTest, ChecksUnreachableBranch)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "if(0){print unknown;}", "unknown");
}

TEST(SemanticAnalyzerTest, ChecksControlConditions)
{
    Segmantic_Analiz analyzer(test_file_name());
    expect_undeclared(analyzer, "if(unknown){print body;}", "unknown");
    expect_undeclared(analyzer, "while(unknown){print body;}", "unknown");
}

TEST(SemanticAnalyzerTest, ChecksNestedExpressions)
{
    Segmantic_Analiz analyzer(test_file_name());
    for (const auto* source : {"print -a;", "print a+1;", "print 1+a;"})
    {
        expect_undeclared(analyzer, source, "a");
    }
}

TEST(SemanticAnalyzerTest, AcceptsIfWithoutElse)
{
    Segmantic_Analiz analyzer(test_file_name());
    const auto program = parse_program("if(1){}");
    EXPECT_NO_THROW(analyzer.segmantic_check(program.get()));
}


TEST(SemanticAnalyzerTest, ReportsVariableLocation)
{
    Segmantic_Analiz analyzer(test_file_name());
    const auto program = parse_program("x=1;\nprint missing;");
    try
    {
        analyzer.segmantic_check(program.get());
        FAIL() << "Ожидалась ошибка для переменной missing";
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();
        EXPECT_NE(message.find(test_file_name() + ":2:7: ошибка:"), std::string::npos) << message;
        EXPECT_NE(message.find("'missing'"), std::string::npos) << message;
    }
}

TEST(SemanticAnalyzerTest, CanAnalyzeSameTreeTwice)
{
    Segmantic_Analiz analyzer(test_file_name());
    const auto program = parse_program("x=1; {print x;}");
    ASSERT_NO_THROW(analyzer.segmantic_check(program.get()));
    EXPECT_NO_THROW(analyzer.segmantic_check(program.get()));
    expect_undeclared(analyzer, "print x;", "x");
}
