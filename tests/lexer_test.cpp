#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include "lexer.hpp"

TEST(LexerTest, EmptySourceProducesEndFile)
{
    {
        std::ofstream file("test_lexer_1.pcl");
        ASSERT_TRUE(file.is_open());

        file << "";
        file.close();
    }

    Lexer lexer("test_lexer_1.pcl");
    auto token = lexer.next_token();

    ASSERT_NE(token, nullptr);
    EXPECT_EQ(token -> token_type, Token_Type::EndFile);
    EXPECT_EQ(token -> token_name, "");
    EXPECT_EQ((token -> location).line, 1);
    EXPECT_EQ((token -> location).column, 1);
    EXPECT_EQ((token -> location).offset, 0);
}

TEST(LexerTest, RecognizesKeywords)
{
    {
        std::ofstream file("test_lexer_2.pcl");
        ASSERT_TRUE(file.is_open());

        file << "if else while print else if";
        file.close();
    }

    Lexer lexer("test_lexer_2.pcl");

    const Token_Type expected_types[] = {
        Token_Type::If,
        Token_Type::Else,
        Token_Type::While,
        Token_Type::Print,
        Token_Type::Else,
        Token_Type::If,
        Token_Type::EndFile
    };

    const std::string expected_names[] = {"if", "else", "while", "print", "else", "if", ""};

    for (std::size_t i = 0; i < std::size(expected_types); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected_types[i]);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }
}

TEST(LexerTest, DistinguishesKeywordsFromIdentifiers)
{
    {
        std::ofstream file("test_lexer_3.pcl");
        ASSERT_TRUE(file.is_open());

        file << "ifelse while2 print_ _if Else";
        file.close();
    }

    Lexer lexer("test_lexer_3.pcl");

    const std::string expected_names[] = {"ifelse", "while2", "print_", "_if", "Else"};

    for (std::size_t i = 0; i < std::size(expected_names); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, Token_Type::Identifier);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }

    const auto end = lexer.next_token();

    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end -> token_type, Token_Type::EndFile);
}

TEST(LexerTest, ReadsIdentifiers)
{
    {
        std::ofstream file("test_lexer_4.pcl");
        ASSERT_TRUE(file.is_open());

        file << "x _name A1 counter_2";
        file.close();
    }

    Lexer lexer("test_lexer_4.pcl");
    const std::string expected_names[] = {"x", "_name", "A1", "counter_2"};

    for (std::size_t i = 0; i < std::size(expected_names); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, Token_Type::Identifier);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }

    const auto end = lexer.next_token();

    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end -> token_type, Token_Type::EndFile);
}

TEST(LexerTest, ReadsCompleteInteger)
{
    {
        std::ofstream file("test_lexer_5.pcl");
        ASSERT_TRUE(file.is_open());

        file << "0 12345 0012 9223372036854775808 999999999999999999999999999999";
        file.close();
    }

    Lexer lexer("test_lexer_5.pcl");
    const std::string expected_names[] = {"0", "12345", "0012", "9223372036854775808", "999999999999999999999999999999"};

    for (std::size_t i = 0; i < std::size(expected_names); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, Token_Type::Integer);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }

    const auto end = lexer.next_token();

    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end -> token_type, Token_Type::EndFile);
}

TEST(LexerTest, TokenizesWithoutWhitespace)
{
    {
        std::ofstream file("test_lexer_6.pcl");
        ASSERT_TRUE(file.is_open());

        file << "x=12+3;";
        file.close();
    }

    Lexer lexer("test_lexer_6.pcl");

    const Token_Type expected_types[] = {
        Token_Type::Identifier,
        Token_Type::Assign,
        Token_Type::Integer,
        Token_Type::Plus,
        Token_Type::Integer,
        Token_Type::Semicolon,
        Token_Type::EndFile
    };
    const std::string expected_names[] = {"x", "=", "12", "+", "3", ";", ""};

    for (std::size_t i = 0; i < std::size(expected_types); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected_types[i]);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }
}

TEST(LexerTest, RecognizesOperatorsAndDelimiters)
{
    const Token_Type expected_types[] = {
        Token_Type::Plus,
        Token_Type::Minus,
        Token_Type::Multiplic,
        Token_Type::Division,
        Token_Type::Percent,
        Token_Type::Assign,
        Token_Type::Equal,
        Token_Type::NotEqual,
        Token_Type::LogicNo,
        Token_Type::LogicAnd,
        Token_Type::LogicOr,
        Token_Type::Less,
        Token_Type::LessEqual,
        Token_Type::Greater,
        Token_Type::GreaterEqual,
        Token_Type::LeftParen,
        Token_Type::RightParen,
        Token_Type::LeftBrace,
        Token_Type::RightBrace,
        Token_Type::Semicolon,
        Token_Type::Question
    };
    const std::string expected_names[] = {
        "+", "-", "*", "/", "%", "=", "==", "!=", "!", "&&", "||", "<", "<=", ">", ">=", "(", ")", "{", "}", ";", "?"
    };

    {
        std::ofstream file("test_lexer_7.pcl");
        ASSERT_TRUE(file.is_open());

        file << "+ - * / % = == != ! && || < <= > >= ( ) { } ; ?";
        file.close();
    }

    Lexer lexer("test_lexer_7.pcl");

    for (std::size_t i = 0; i < std::size(expected_types); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected_types[i]);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }

    const auto end = lexer.next_token();

    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end -> token_type, Token_Type::EndFile);

    for (std::size_t i = 0; i < std::size(expected_types); ++i)
    {
        SCOPED_TRACE(expected_names[i]);

        {
            std::ofstream file("test_lexer_7_end.pcl");
            ASSERT_TRUE(file.is_open());

            file << expected_names[i];
            file.close();
        }

        Lexer single_lexer("test_lexer_7_end.pcl");
        const auto token = single_lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected_types[i]);
        EXPECT_EQ(token -> token_name, expected_names[i]);

        const auto single_end = single_lexer.next_token();

        ASSERT_NE(single_end, nullptr);
        EXPECT_EQ(single_end -> token_type, Token_Type::EndFile);
    }
}

TEST(LexerTest, ReadsLongestAvailableOperator)
{
    {
        std::ofstream file("test_lexer_8.pcl");
        ASSERT_TRUE(file.is_open());

        file << "a===b!==c<=d>=e";
        file.close();
    }

    Lexer lexer("test_lexer_8.pcl");
    const Token_Type expected_types[] = {
        Token_Type::Identifier,
        Token_Type::Equal,
        Token_Type::Assign,
        Token_Type::Identifier,
        Token_Type::NotEqual,
        Token_Type::Assign,
        Token_Type::Identifier,
        Token_Type::LessEqual,
        Token_Type::Identifier,
        Token_Type::GreaterEqual,
        Token_Type::Identifier,
        Token_Type::EndFile
    };
    const std::string expected_names[] = {"a", "==", "=", "b", "!=", "=", "c", "<=", "d", ">=", "e", ""};

    for (std::size_t i = 0; i < std::size(expected_types); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected_types[i]);
        EXPECT_EQ(token -> token_name, expected_names[i]);
    }
}

TEST(LexerTest, KeepsMinusSeparateFromInteger)
{
    {
        std::ofstream file("test_lexer_9.pcl");
        ASSERT_TRUE(file.is_open());

        file << "-52";
        file.close();
    }

    Lexer lexer("test_lexer_9.pcl");

    const auto minus = lexer.next_token();
    const auto integer = lexer.next_token();
    const auto end = lexer.next_token();

    ASSERT_NE(minus, nullptr);
    EXPECT_EQ(minus -> token_type, Token_Type::Minus);
    EXPECT_EQ(minus -> token_name, "-");

    ASSERT_NE(integer, nullptr);
    EXPECT_EQ(integer -> token_type, Token_Type::Integer);
    EXPECT_EQ(integer -> token_name, "52");

    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end -> token_type, Token_Type::EndFile);
}

TEST(LexerTest, TracksTokenLocations)
{
    std::ofstream file("test_lexer_11.pcl");
    ASSERT_TRUE(file.is_open());

    file << "x\n  print x;// hi\n\tname!=2;";
    file.close();

    Lexer lexer("test_lexer_11.pcl");

    const std::size_t lines[]   = {1, 2, 2, 2, 3, 3, 3, 3, 3};
    const std::size_t columns[] = {1, 3, 9, 10, 2, 6, 8, 9, 10};
    const std::size_t offsets[] = {0, 4, 10, 11, 19, 23, 25, 26, 27};

    for (std::size_t i = 0; i < std::size(lines); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ((token -> location).line, lines[i]);
        EXPECT_EQ((token -> location).column, columns[i]);
        EXPECT_EQ((token -> location).offset, offsets[i]);
    }
}

TEST(LexerTest, RepeatedEndFileIsStable)
{
    std::ofstream file("test_lexer_12.pcl");
    ASSERT_TRUE(file.is_open());

    file << "x";
    file.close();

    Lexer lexer("test_lexer_12.pcl");

    const auto token = lexer.next_token();

    ASSERT_NE(token, nullptr);
    EXPECT_EQ(token -> token_type, Token_Type::Identifier);
    EXPECT_EQ(token -> token_name, "x");

    for (std::size_t i = 0; i < 3; ++i)
    {
        const auto end = lexer.next_token();

        ASSERT_NE(end, nullptr);
        EXPECT_EQ(end -> token_type, Token_Type::EndFile);
        EXPECT_EQ(end -> token_name, "");
        EXPECT_EQ((end -> location).line, 1);
        EXPECT_EQ((end -> location).column, 2);
        EXPECT_EQ((end -> location).offset, 1);
    }
}

TEST(LexerTest, PreviousTokenRemainsUnchanged)
{
    {
        std::ofstream file("test_lexer_13.pcl");
        ASSERT_TRUE(file.is_open());

        file << "first second";
        file.close();
    }

    Lexer lexer("test_lexer_13.pcl");
    const auto first = lexer.next_token();
    const auto second = lexer.next_token();
    const auto end = lexer.next_token();

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first -> token_type, Token_Type::Identifier);
    EXPECT_EQ(first -> token_name, "first");
    EXPECT_EQ((first -> location).line, 1);
    EXPECT_EQ((first -> location).column, 1);
    EXPECT_EQ((first -> location).offset, 0);

    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second -> token_type, Token_Type::Identifier);
    EXPECT_EQ(second -> token_name, "second");
    EXPECT_EQ((second -> location).column, 7);

    ASSERT_NE(end, nullptr);
    EXPECT_EQ(end -> token_type, Token_Type::EndFile);
}

TEST(LexerTest, RejectsUnknownCharacter)
{
    const char symbols[] = {'@', '&', '|', '.', ',', '[', '"'};

    for (std::size_t i = 0; i < std::size(symbols); ++i)
    {
        std::ofstream file("test_lexer_14.pcl");
        ASSERT_TRUE(file.is_open());

        file << symbols[i];
        file.close();

        Lexer lexer("test_lexer_14.pcl");

        EXPECT_THROW(lexer.next_token(), std::runtime_error);
    }
}

TEST(LexerTest, RejectsMalformedInteger)
{
    const std::string sources[] = {"123abc", "12_name", "0x10"};

    for (std::size_t i = 0; i < std::size(sources); ++i)
    {
        SCOPED_TRACE(sources[i]);

        {
            std::ofstream file("test_lexer_15.pcl");
            ASSERT_TRUE(file.is_open());

            file << "\n  " << sources[i];
            file.close();
        }

        Lexer lexer("test_lexer_15.pcl");

        try
        {
            lexer.next_token();
            FAIL() << "Ожидалась ошибка для литерала " << sources[i];
        }
        catch (const std::runtime_error& error)
        {
            EXPECT_STREQ(error.what(), "test_lexer_15.pcl:2:3: ошибка: неверный целочисленный литерал");
        }
    }
}

TEST(LexerTest, RecognizesAdjacentLogicalOperatorsAndTheirLocations)
{
    std::ofstream file("test_lexer_16.pcl");
    ASSERT_TRUE(file.is_open());
    file << "!x\na!=b&&c||d";
    file.close();

    Lexer lexer("test_lexer_16.pcl");
    const Token_Value expected_tokens[] = {
        {Token_Type::LogicNo,    "!",  {1, 1, 0}},
        {Token_Type::Identifier, "x",  {1, 2, 1}},
        {Token_Type::Identifier, "a",  {2, 1, 3}},
        {Token_Type::NotEqual,   "!=", {2, 2, 4}},
        {Token_Type::Identifier, "b",  {2, 4, 6}},
        {Token_Type::LogicAnd,   "&&", {2, 5, 7}},
        {Token_Type::Identifier, "c",  {2, 7, 9}},
        {Token_Type::LogicOr,    "||", {2, 8, 10}},
        {Token_Type::Identifier, "d",  {2, 10, 12}},
        {Token_Type::EndFile,    "",   {2, 11, 13}}
    };

    for (const auto& expected : expected_tokens)
    {
        SCOPED_TRACE(expected.token_name);
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected.token_type);
        EXPECT_EQ(token -> token_name, expected.token_name);
        EXPECT_EQ(token -> location.line, expected.location.line);
        EXPECT_EQ(token -> location.column, expected.location.column);
        EXPECT_EQ(token -> location.offset, expected.location.offset);
    }
}

TEST(LexerTest, ReportsLexicalErrorLocation)
{
    {
        std::ofstream file("test_lexer_17.pcl");
        ASSERT_TRUE(file.is_open());

        file << "x=1;\nprint @;";
        file.close();
    }

    Lexer lexer("test_lexer_17.pcl");
    const Token_Type expected_types[] = {
        Token_Type::Identifier,
        Token_Type::Assign,
        Token_Type::Integer,
        Token_Type::Semicolon,
        Token_Type::Print
    };

    for (std::size_t i = 0; i < std::size(expected_types); ++i)
    {
        const auto token = lexer.next_token();

        ASSERT_NE(token, nullptr);
        EXPECT_EQ(token -> token_type, expected_types[i]);
    }

    try
    {
        lexer.next_token();
        FAIL() << "Ожидалась ошибка для символа @";
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();

        EXPECT_NE(message.find("test_lexer_17.pcl:2:7: ошибка:"), std::string::npos) << message;
        EXPECT_NE(message.find('@'), std::string::npos);
    }
}

TEST(LexerTest, ReportsIncompleteLogicalOperatorLocation)
{
    const std::string file_name = "test_lexer_18.pcl";
    for (const std::string operation : {"&&", "||"})
    {
        SCOPED_TRACE(operation);
        std::ofstream file(file_name);
        ASSERT_TRUE(file.is_open());
        file << "// comment\n  " << operation.front() << " x";
        file.close();

        Lexer lexer(file_name);
        try
        {
            lexer.next_token();
            FAIL() << "Ожидалась ошибка для одиночного " << operation.front();
        }
        catch (const std::runtime_error& error)
        {
            const std::string expected = file_name + ":2:3: ошибка: ожидался второй '" + operation.front() +
                "' для оператора '" + operation + "'";
            EXPECT_EQ(error.what(), expected);
        }
    }
}
