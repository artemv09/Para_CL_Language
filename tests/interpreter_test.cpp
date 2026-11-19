#include <gtest/gtest.h>

#include "interpreter.hpp"
#include "semantic_analyzer.hpp"
#include "test_program.hpp"

namespace
{
    const std::string minimum = "-9223372036854775808";
    const std::string maximum = "9223372036854775807";

    std::unique_ptr<Block_Stmt> checked_program(const std::string& source)
    {
        auto program = parse_program(source);
        Segmantic_Analiz analyzer(test_file_name());
        analyzer.segmantic_check(program.get());
        return program;
    }

    std::string run_program(const std::string& source, const std::string& input = "")
    {
        const auto program = checked_program(source);
        Test_Streams streams;
        streams.input.str(input);
        
        Interpreter interpreter;
        interpreter.run_code(program.get());
        return streams.output.str();
    }

    void expect_runtime_error(const std::string& source, const std::string& expected, const std::string& input = "")
    {
        SCOPED_TRACE(source + " (ввод: " + input + ")");
        const auto program = checked_program(source);
        Test_Streams streams;
        streams.input.str(input);

        Interpreter interpreter;
        try
        {
            interpreter.run_code(program.get());
            FAIL() << "Ожидалась ошибка исполнения";
        }
        catch (const std::runtime_error& error)
        {
            const std::string message = error.what();
            EXPECT_NE(message.find(expected), std::string::npos) << message;
        }
    }
}

TEST(InterpreterTest, EmptyProgramProducesNoOutput)
{
    EXPECT_EQ(run_program(""), "");
    EXPECT_EQ(run_program("{}"), "");
}

TEST(InterpreterTest, PrintsInteger)
{
    EXPECT_EQ(run_program("print 42;"), "42\n");
}

TEST(InterpreterTest, EvaluatesArithmetic)
{
    EXPECT_EQ(run_program("print 7+3; print 7-3; print 7*3; print 7/3; print 7%3;"), "10\n4\n21\n2\n1\n");
}

TEST(InterpreterTest, RespectsPrecedenceAndAssociativity)
{
    EXPECT_EQ(run_program("print 2+3*4; print (2+3)*4; print 10-3-2;"), "14\n20\n5\n");
}

TEST(InterpreterTest, EvaluatesUnaryOperators)
{
    EXPECT_EQ(run_program("print +5; print -5; print -(-5);"), "5\n-5\n5\n");
}

TEST(InterpreterTest, UsesTruncatingIntegerDivision)
{
    EXPECT_EQ(run_program("print -7/3; print -7%3; print 7/-3; print 7%-3;"), "-2\n-1\n-2\n1\n");
}

TEST(InterpreterTest, ComparisonsProduceZeroOrOne)
{
    EXPECT_EQ(run_program("print 1<2; print 2<2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 2<=2; print 3<=2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 3>2; print 2>2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 2>=2; print 1>=2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 2==2; print 1==2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 1!=2; print 2!=2;"), "1\n0\n");
}

TEST(InterpreterTest, ReadsAssignedValue)
{
    EXPECT_EQ(run_program("x=5; print x; x=x+2; print x;"), "5\n7\n");
}

TEST(InterpreterTest, UpdatesVariableInOuterScope)
{
    EXPECT_EQ(run_program("x=1; {{x=x+1;}} print x;"), "2\n");
}

TEST(InterpreterTest, ExecutesOnlySelectedBranch)
{
    EXPECT_EQ(run_program("if(1){print 7;}else{print 1/0;}"), "7\n");
    EXPECT_EQ(run_program("if(0){print 1/0;}else{print 8;}"), "8\n");
}

TEST(InterpreterTest, SelectsFirstMatchingElseIf)
{
    EXPECT_EQ(run_program("if(0){print 0;}else if(1){print 1;}else if(1){print 2;}else{print 3;}"), "1\n");
    EXPECT_EQ(run_program("if(0){print 0;}else if(0){print 1;}else{print 3;}"), "3\n");
}

TEST(InterpreterTest, TreatsNonzeroConditionAsTrue)
{
    EXPECT_EQ(run_program("if(-2){print 1;} if(42){print 2;} if(0){print 3;}"), "1\n2\n");
}

TEST(InterpreterTest, SkipsWhileWithFalseCondition)
{
    EXPECT_EQ(run_program("while(0){print 1/0;} print 2;"), "2\n");
}

TEST(InterpreterTest, ReevaluatesWhileCondition)
{
    EXPECT_EQ(run_program("i=3; while(i>0){print i; i=i-1;} print i;"), "3\n2\n1\n0\n");
}

TEST(InterpreterTest, ExecutesNestedLoops)
{
    const std::string source = "i=0; while(i<2){j=0; while(j<2){print i*10+j; j=j+1;} i=i+1;}";
    EXPECT_EQ(run_program(source), "0\n1\n10\n11\n");
}

TEST(InterpreterTest, ReadsInputInLeftToRightOrder)
{
    EXPECT_EQ(run_program("print ?-?;", "10 3"), "7\n");
}

TEST(InterpreterTest, DoesNotReadInputFromSkippedBranch)
{
    EXPECT_EQ(run_program("if(0){x=?;} print ?;", "9"), "9\n");
}

TEST(InterpreterTest, RejectsInvalidInput)
{
    expect_runtime_error("x=?;", "ошибка ввода", "abc");
    expect_runtime_error("x=?;", "ошибка ввода", "9223372036854775808");
}

TEST(InterpreterTest, RejectsInputAtEndOfStream)
{
    expect_runtime_error("x=?;", "ошибка ввода");
    expect_runtime_error("x=?; y=?;", "ошибка ввода", "1");
}

TEST(InterpreterTest, RejectsDivisionByZero)
{
    expect_runtime_error("print 1/0;", "деление на ноль");
    expect_runtime_error("x=2; print 5/(x-2);", "деление на ноль");
}

TEST(InterpreterTest, RejectsModuloByZero)
{
    expect_runtime_error("print 1%0;", "деление на ноль");
}

TEST(InterpreterTest, RejectsAdditionOverflow)
{
    expect_runtime_error("print ?+1;", "переполнение", maximum);
    expect_runtime_error("print ?+(-1);", "переполнение", minimum);
}

TEST(InterpreterTest, RejectsSubtractionOverflow)
{
    expect_runtime_error("print ?-1;", "переполнение", minimum);
    expect_runtime_error("print ?-(-1);", "переполнение", maximum);
}

TEST(InterpreterTest, RejectsMultiplicationOverflow)
{
    expect_runtime_error("print ?*2;", "переполнение", maximum);
    expect_runtime_error("print ?*2;", "переполнение", minimum);
    expect_runtime_error("print 2*?;", "переполнение", minimum);
    expect_runtime_error("print ?*(-1);", "переполнение", minimum);
}

TEST(InterpreterTest, RejectsNegationOverflow)
{
    expect_runtime_error("x=?; print -x;", "переполнение", minimum);
}

TEST(InterpreterTest, RejectsDivisionOverflow)
{
    expect_runtime_error("x=?; print x/-1;", "переполнение", minimum);
}

TEST(InterpreterTest, RejectsModuloOverflow)
{
    expect_runtime_error("x=?; print x%-1;", "переполнение", minimum);
}

TEST(InterpreterTest, AcceptsArithmeticBoundaryResults)
{
    const std::string source = "max=?; min=?; print max+0; print min-0; print min/1; print min*1;"
                               "print max*0; print min%1; print (-max)*(-1); print min-min;";
    const std::string expected = maximum + "\n" + minimum + "\n" + minimum + "\n" + minimum + "\n0\n0\n" + maximum + "\n0\n";

    EXPECT_EQ(run_program(source, maximum + " " + minimum), expected);
}

TEST(InterpreterTest, RestoresScopesAfterRuntimeFailure)
{
    const auto failing = checked_program("x=1; {print 1/0;}");
    const auto valid = checked_program("{x=2;} x=3; print x;");
    const auto undeclared = parse_program("print x;");
    Test_Streams streams;
    Interpreter interpreter;

    ASSERT_THROW(interpreter.run_code(failing.get()), std::runtime_error);
    ASSERT_NO_THROW(interpreter.run_code(valid.get()));
    EXPECT_EQ(streams.output.str(), "3\n");
    EXPECT_THROW(interpreter.run_code(undeclared.get()), std::runtime_error);
}

TEST(InterpreterTest, RunsSameTreeTwice)
{
    const auto program = checked_program("x=0; x=x+1; print x;");
    Test_Streams streams;
    Interpreter interpreter;

    interpreter.run_code(program.get());
    interpreter.run_code(program.get());
    EXPECT_EQ(streams.output.str(), "1\n1\n");
}

TEST(InterpreterTest, RunsFibonacciProgram)
{
    const std::string source = "n=?; a=0; b=1; i=0; while(i<n){print a; next=a+b; a=b; b=next; i=i+1;}";
    EXPECT_EQ(run_program(source, "6"), "0\n1\n1\n2\n3\n5\n");
    EXPECT_EQ(run_program(source, "0"), "");
}
