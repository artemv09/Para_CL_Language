#include "diagnostic.hpp"

std::string Diagnostic::format(const std::string& file_name) const
{
    return file_name + ":" + std::to_string(location.line) + ":" + std::to_string(location.column) + ": ошибка: " + message;
}
