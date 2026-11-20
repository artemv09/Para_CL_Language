#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "test_program.hpp"

namespace
{
    namespace fs = std::filesystem;

    struct Run_Result
    {
        int exit_code;
        std::string stdout_text;
        std::string stderr_text;
    };

    class Temporary_Directory
    {
        public:
            fs::path path;

            Temporary_Directory()
            {
                const auto name = fs::path(test_file_name()).stem().string();
                for (unsigned i = 0; ; ++i)
                {
                    path = "paracl-" + name + "-" + std::to_string(i);
                    if (fs::create_directory(path))
                    {
                        return;
                    }
                }
            }

            ~Temporary_Directory()
            {
                std::error_code error;
                fs::remove_all(path, error);

                if (error)
                {
                    ADD_FAILURE() << "Не удалось удалить " << path << ": " << error.message();
                }
            }

            Temporary_Directory(const Temporary_Directory&) = delete;
            Temporary_Directory& operator=(const Temporary_Directory&) = delete;
    };

    void write_file(const fs::path& path, const std::string& text)
    {
        std::ofstream file(path);
        file << text;
        file.close();

        if (!file)
        {
            throw std::runtime_error("Не удалось записать " + path.string());
        }
    }

    std::string read_file(const fs::path& path)
    {
        std::ifstream file(path);
        if (!file)
        {
            throw std::runtime_error("Не удалось открыть " + path.string());
        }

        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    Run_Result run_paracl(const std::string& file_name, const std::string& input)
    {
        const Temporary_Directory directory;
        const auto program = fs::path(PARACL_E2E_DIR) / file_name;

        write_file(directory.path / "program_path", program.generic_string());
        write_file(directory.path / "stdin", input);
        write_file(directory.path / "run.cmake", R"cmake(
            file(READ program_path program)
            execute_process(
                COMMAND [==[)cmake" PARACL_EXECUTABLE R"cmake(]==] "${program}"
                INPUT_FILE stdin
                OUTPUT_FILE stdout
                ERROR_FILE stderr
                RESULT_VARIABLE exit_code
                TIMEOUT 4
            )
            if(NOT exit_code MATCHES "^[0-9]+$")
                message(FATAL_ERROR "paracl: ${exit_code}")
            endif()
            file(WRITE exit_code "${exit_code}")
        )cmake");

        const auto command = "cd \"" + directory.path.string() + "\" && cmake -P run.cmake";

        if (std::system(command.c_str()) != 0)
        {
            throw std::runtime_error("Не удалось завершить запуск paracl");
        }

        return {std::stoi(read_file(directory.path / "exit_code")), read_file(directory.path / "stdout"), read_file(directory.path / "stderr")};
    }

    void expect_success(const Run_Result& result, const std::string& expected_output)
    {
        EXPECT_EQ(result.exit_code, 0);
        EXPECT_EQ(result.stdout_text, expected_output);
        EXPECT_EQ(result.stderr_text, "");
    }

    void expect_error(const Run_Result& result, const std::string& expected_output, const std::string& diagnostic)
    {
        EXPECT_NE(result.exit_code, 0);
        EXPECT_EQ(result.stdout_text, expected_output);
        EXPECT_NE(result.stderr_text.find(diagnostic), std::string::npos) << "Ожидалось: " << diagnostic << "\nПолучено: " << result.stderr_text;
    }
}

TEST(E2E, NestedAssignmentValues)
{
    expect_success(run_paracl("001.pcl", ""), "1\n2\n4\n");
}

TEST(E2E, EmptyStatementBeforeWhile)
{
    expect_success(run_paracl("003.pcl", "1\n0\n"), "");
}

TEST(E2E, FibonacciZeroIterations)
{
    expect_success(run_paracl("fibonacci.pcl", "0\n"), "1\n");
}

TEST(E2E, FibonacciFiveIterations)
{
    expect_success(run_paracl("fibonacci.pcl", "5\n"), "8\n");
}

TEST(E2E, ElseIfSelectionIf)
{
    expect_success(run_paracl("branch_selection.pcl", "0\n"), "10\n");
}

TEST(E2E, ElseIfSelectionFirstElseIf)
{
    expect_success(run_paracl("branch_selection.pcl", "1\n"), "20\n");
}

TEST(E2E, ElseIfSelectionSecondElseIf)
{
    expect_success(run_paracl("branch_selection.pcl", "2\n"), "30\n");
}

TEST(E2E, ElseIfSelectionElse)
{
    expect_success(run_paracl("branch_selection.pcl", "3\n"), "40\n");
}

TEST(E2E, OuterVariableUpdatedInBlock)
{
    expect_success(run_paracl("block_scope.pcl", ""), "3\n7\n");
}

TEST(E2E, AssignmentReadsInputOnce)
{
    expect_success(run_paracl("assignment_input.pcl", "7\n9\n"), "7\n7\n9\n");
}

TEST(E2E, VariableOutsideScope)
{
    expect_error(run_paracl("undeclared_variable.pcl", ""), "", "переменная 'y' не объявлена");
}

TEST(E2E, DivisionByZero)
{
    expect_error(run_paracl("division_by_zero.pcl", ""), "42\n", "деление на ноль");
}
