#ifndef DIAGNOSTIC_HPP_
#define DIAGNOSTIC_HPP_

#include <cstddef>
#include <string>

struct Source_Location
{
    std::size_t line = 1;
    std::size_t column = 1;
    std::size_t offset = 0;
};

struct Diagnostic
{
    Source_Location location;
    std::string message;

    std::string format(const std::string& file_name) const;
};

#endif
