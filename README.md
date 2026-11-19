# ParaCL

ParaCL — интерпретатор языка с целыми числами, переменными, арифметикой и сравнениями. Он поддерживает ветвления `if / else if / else`, циклы `while` и блоки со своей областью видимости. Оператор `?` читает число, а `print` выводит значение выражения.

## Сборка и запуск

Нужны CMake 3.16 или новее и компилятор C++20. Выполните команды из корня проекта.

```sh
cmake -S . -B build/run -DBUILD_RUN_PROGRAM=ON -DBUILD_TESTING=OFF
cmake --build build/run --parallel
```

Сохраните эту программу в файл `example.pcl`.

```text
x = ?;
print x * x;
```

```sh
./build/run/paracl example.pcl
```

Введите `7` и нажмите Enter — программа выведет `49`. Перед исполнением интерпретатор проверяет программу и сохраняет её синтаксическое дерево (AST) в формате DOT. Для этого примера файл появится по пути `graphviz/example.pcl.dot` относительно текущего каталога.

## Тесты

В проекте есть тесты лексера, парсера, семантического анализатора и интерпретатора. При первой настройке тестовой сборки CMake скачивает GoogleTest.

```sh
cmake -S . -B build/tests -DBUILD_RUN_PROGRAM=OFF -DBUILD_TESTING=ON
cmake --build build/tests --parallel
(cd build/tests && ctest --output-on-failure)
```
