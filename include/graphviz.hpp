#ifndef GRAPHVIZ_HPP_
#define GRAPHVIZ_HPP_

#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <string>

class Node;

class Graphviz_Writer
{
    private:
        std::ostream& output;
        std::size_t next_id = 0;

    public:
        explicit Graphviz_Writer(std::ostream& output);
        std::size_t add_node(const std::string& label);
        void add_edge(std::size_t parent, std::size_t child);
        void write(const Node& root);
};

std::filesystem::path dump_graphviz(const Node& root, const std::filesystem::path& source);

#endif
