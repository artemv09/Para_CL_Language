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

        Interpreter interpreter(test_file_name());
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

TEST(InterpreterTest, EmptyProgramNoOutput)
{
    EXPECT_EQ(run_program(""), "");
    EXPECT_EQ(run_program("{}"), "");
}

TEST(InterpreterTest, PrintsNumber)
{
    EXPECT_EQ(run_program("print 42;"), "42\n");
}

TEST(InterpreterTest, BasicArithmetic)
{
    EXPECT_EQ(run_program("print 7+3; print 7-3; print 7*3; print 7/3; print 7%3;"), "10\n4\n21\n2\n1\n");
}

TEST(InterpreterTest, ArithmeticOrder)
{
    EXPECT_EQ(run_program("print 2+3*4; print (2+3)*4; print 10-3-2;"), "14\n20\n5\n");
}

TEST(InterpreterTest, UnaryPlusMinus)
{
    EXPECT_EQ(run_program("print +5; print -5; print -(-5);"), "5\n-5\n5\n");
}

TEST(InterpreterTest, LogicReturns0Or1)
{
    const std::string source = "print !0; print !(-3); print !!7; print 2 && 5; print 0 || -3;";
    EXPECT_EQ(run_program(source), "1\n0\n1\n1\n1\n");
}

TEST(InterpreterTest, LogicSkipsUnusedInput)
{
    EXPECT_EQ(run_program("print 0 && ?; print 1 || ?; print ?;", "9"), "0\n1\n9\n");
}

TEST(InterpreterTest, LogicReadsNeededInput)
{
    EXPECT_EQ(run_program("print 1 && ?; print 0 || ?;", "0 7"), "0\n1\n");
}

TEST(InterpreterTest, LogicReadsOnceInOrder)
{
    const std::string source = "print !?; print ? && ?; print ? || ?; print ?;";
    EXPECT_EQ(run_program(source, "0 2 0 0 -3 9"), "1\n0\n1\n9\n");
}

TEST(InterpreterTest, LogicCanSkipDivisionByZero)
{
    EXPECT_EQ(run_program("print 0 && (1 / 0); print 1 || (1 / 0);"), "0\n1\n");
    expect_runtime_error("print 1 && (1 / 0);", "деление на ноль");
    expect_runtime_error("print 0 || (1 / 0);", "деление на ноль");
}

TEST(InterpreterTest, LogicCanSkipAssignment)
{
    const std::string source = "x=2; print 0 && (x=3); print 1 || (x=4); print x;"
                               "print 1 && (x=5); print x; print 0 || (x=6); print x;";
    EXPECT_EQ(run_program(source), "0\n1\n2\n1\n5\n1\n6\n");
}

TEST(InterpreterTest, DivisionRoundsTowardZero)
{
    EXPECT_EQ(run_program("print -7/3; print -7%3; print 7/-3; print 7%-3;"), "-2\n-1\n-2\n1\n");
}

TEST(InterpreterTest, ComparisonsReturn0Or1)
{
    EXPECT_EQ(run_program("print 1<2; print 2<2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 2<=2; print 3<=2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 3>2; print 2>2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 2>=2; print 1>=2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 2==2; print 1==2;"), "1\n0\n");
    EXPECT_EQ(run_program("print 1!=2; print 2!=2;"), "1\n0\n");
}

TEST(InterpreterTest, AssignAndRead)
{
    EXPECT_EQ(run_program("x=5; print x; x=x+2; print x;"), "5\n7\n");
}

TEST(InterpreterTest, UpdatesOuterVariable)
{
    EXPECT_EQ(run_program("x=1; {{x=x+1;}} print x;"), "2\n");
}

TEST(InterpreterTest, RunsChosenBranch)
{
    EXPECT_EQ(run_program("if(1){print 7;}else{print 1/0;}"), "7\n");
    EXPECT_EQ(run_program("if(0){print 1/0;}else{print 8;}"), "8\n");
}

TEST(InterpreterTest, ElseMatchesInnerIf)
{
    EXPECT_EQ(run_program("if (1) if (0) print 3; else print 2;"), "2\n");
    EXPECT_EQ(run_program("if (0) if (0) print 3; else print 2;"), "");
}

TEST(InterpreterTest, UnbracedIfSharesScope)
{
    EXPECT_EQ(run_program("if(1) x=3; if(0) x=4; else x=x+2; print x;"), "5\n");
}

TEST(InterpreterTest, FirstTrueElseIf)
{
    EXPECT_EQ(run_program("if(0){print 0;}else if(1){print 1;}else if(1){print 2;}else{print 3;}"), "1\n");
    EXPECT_EQ(run_program("if(0){print 0;}else if(0){print 1;}else{print 3;}"), "3\n");
}

TEST(InterpreterTest, NonzeroIsTrue)
{
    EXPECT_EQ(run_program("if(-2){print 1;} if(42){print 2;} if(0){print 3;}"), "1\n2\n");
}

TEST(InterpreterTest, WhileSkipsIfFalse)
{
    EXPECT_EQ(run_program("while(0){print 1/0;} print 2;"), "2\n");
}

TEST(InterpreterTest, WhileRechecksCondition)
{
    EXPECT_EQ(run_program("i=3; while(i>0){print i; i=i-1;} print i;"), "3\n2\n1\n0\n");
}

TEST(InterpreterTest, NestedWhileLoops)
{
    const std::string source = "i=0; while(i<2){j=0; while(j<2){print i*10+j; j=j+1;} i=i+1;}";
    EXPECT_EQ(run_program(source), "0\n1\n10\n11\n");
}

TEST(InterpreterTest, ReadsInputLeftToRight)
{
    EXPECT_EQ(run_program("print ?-?;", "10 3"), "7\n");
}

TEST(InterpreterTest, FalseIfSkipsInput)
{
    EXPECT_EQ(run_program("if(0){x=?;} print ?;", "9"), "9\n");
}

TEST(InterpreterTest, InvalidInputFails)
{
    expect_runtime_error("x=?;", "ошибка ввода", "abc");
    expect_runtime_error("x=?;", "ошибка ввода", "9223372036854775808");
}

TEST(InterpreterTest, MissingInputFails)
{
    expect_runtime_error("x=?;", "ошибка ввода");
    expect_runtime_error("x=?; y=?;", "ошибка ввода", "1");
}

TEST(InterpreterTest, DivisionByZero)
{
    expect_runtime_error("print 1/0;", "деление на ноль");
    expect_runtime_error("x=2; print 5/(x-2);", "деление на ноль");
}

TEST(InterpreterTest, ModuloByZero)
{
    expect_runtime_error("print 1%0;", test_file_name() + ":1:8: ошибка: деление на ноль");
}

TEST(InterpreterTest, MissingVariableLocation)
{
    expect_runtime_error("if(0) x=1;\nprint x;", test_file_name() + ":2:7: ошибка: Переменная 'x' не объявлена");
}

TEST(InterpreterTest, AdditionOverflow)
{
    expect_runtime_error("print ?+1;", test_file_name() + ":1:8: ошибка: переполнение int64_t при сложении", maximum);
    expect_runtime_error("print ?+(-1);", "переполнение", minimum);
}

TEST(InterpreterTest, SubtractionOverflow)
{
    expect_runtime_error("print ?-1;", "переполнение", minimum);
    expect_runtime_error("print ?-(-1);", "переполнение", maximum);
}

TEST(InterpreterTest, MultiplicationOverflow)
{
    expect_runtime_error("print ?*2;", "переполнение", maximum);
    expect_runtime_error("print ?*2;", "переполнение", minimum);
    expect_runtime_error("print 2*?;", "переполнение", minimum);
    expect_runtime_error("print ?*(-1);", "переполнение", minimum);
}

TEST(InterpreterTest, NegationOverflow)
{
    expect_runtime_error("x=?; print -x;", test_file_name() + ":1:12: ошибка: переполнение int64_t при смене знака", minimum);
}

TEST(InterpreterTest, DivisionOverflow)
{
    expect_runtime_error("x=?; print x/-1;", "переполнение", minimum);
}

TEST(InterpreterTest, ModuloOverflow)
{
    expect_runtime_error("x=?; print x%-1;", "переполнение", minimum);
}

TEST(InterpreterTest, MathAtInt64Limits)
{
    const std::string source = "max=?; min=?; print max+0; print min-0; print min/1; print min*1;"
                               "print max*0; print min%1; print (-max)*(-1); print min-min;";
    const std::string expected = maximum + "\n" + minimum + "\n" + minimum + "\n" + minimum + "\n0\n0\n" + maximum + "\n0\n";

    EXPECT_EQ(run_program(source, maximum + " " + minimum), expected);
}

TEST(InterpreterTest, ClearsScopesAfterError)
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

TEST(InterpreterTest, RunsTreeTwice)
{
    const auto program = checked_program("x=0; x=x+1; print x;");
    Test_Streams streams;
    Interpreter interpreter;

    interpreter.run_code(program.get());
    interpreter.run_code(program.get());
    EXPECT_EQ(streams.output.str(), "1\n1\n");
}

TEST(InterpreterTest, PrintsFibonacci)
{
    const std::string source = "n=?; a=0; b=1; i=0; while(i<n){print a; next=a+b; a=b; b=next; i=i+1;}";
    EXPECT_EQ(run_program(source, "6"), "0\n1\n1\n2\n3\n5\n");
    EXPECT_EQ(run_program(source, "0"), "");
}
