# ParaCL

Сборка и вывод AST в Graphviz:

```sh
cmake -S . -B build
cmake --build build
./build/paracl --dump-ast program.pcl
dot -Tsvg graphviz/program.pcl.dot -o graphviz/program.pcl.svg
```

Каталог `graphviz/` создаётся в текущей рабочей директории. Дерево сохраняется
в `<имя исходного файла>.dot`; повторный запуск перезаписывает этот файл.
Для преобразования DOT в SVG нужна установленная программа Graphviz (`dot`).
Экспорт показывает структуру AST после синтаксического разбора; семантический
анализ пока не реализован.
