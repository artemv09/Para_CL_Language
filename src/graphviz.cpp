#include "graphviz.hpp"
#include "ast.hpp"

#include <fstream>
#include <ostream>
#include <stdexcept>

Graphviz_Writer::Graphviz_Writer(std::ostream& output): output(output)
{
}

std::size_t Graphviz_Writer::add_node(const std::string& label)
{
    const auto id = next_id++;
    output << "  node" << id << " [label=\"";
    for (char character : label)
    {
        switch (character)
        {
            case '\\': output << "\\\\"; break;
            case '"': output << "\\\""; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            default: output << character; break;
        }
    }
    output << "\"];\n";
    return id;
}

void Graphviz_Writer::add_edge(std::size_t parent, std::size_t child)
{
    output << "  node" << parent << " -> node" << child << ";\n";
}

void Graphviz_Writer::write(const Node& root)
{
    next_id = 0;
    output << "digraph AST {\n  node [shape=box];\n  ordering=out;\n";
    root.write_graphviz(*this);
    output << "}\n";
    if (!output)
    {
        throw std::runtime_error("Не удалось записать дерево Graphviz");
    }
}

std::filesystem::path dump_graphviz(const Node& root, const std::filesystem::path& source)
{
    const std::filesystem::path directory = "graphviz";
    std::filesystem::create_directories(directory);
    const auto path = directory / (source.filename().string() + ".dot");
    std::ofstream file(path);
    if (!file)
    {
        throw std::runtime_error("Не удалось открыть " + path.string());
    }
    Graphviz_Writer(file).write(root);
    file.close();
    if (!file)
    {
        throw std::runtime_error("Не удалось сохранить " + path.string());
    }
    return path;
}
