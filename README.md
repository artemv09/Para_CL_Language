# ParaCL

ParaCL — интерпретатор языка с целыми числами, переменными, арифметикой и сравнениями. Он поддерживает ветвления `if / else if / else`, циклы `while` и блоки со своей областью видимости. Оператор `?` читает число, а `print` выводит значение выражения.

Нужны CMake 3.16 или новее и компилятор C++20. Команды для обоих режимов выполняйте из корня проекта.

## Запуск программы

Соберите интерпретатор в каталоге `build/run`.

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

## Запуск всех тестов

Настройте тестовую сборку в каталоге `build/tests`. Опция `BUILD_TESTING` подключает GoogleTest, а `BUILD_RUN_PROGRAM` собирает `paracl`, который сквозные тесты запускают на файлах `.pcl`.

```sh
cmake -S . -B build/tests -DBUILD_TESTING=ON -DBUILD_RUN_PROGRAM=ON
cmake --build build/tests --parallel
(cd build/tests && ctest --output-on-failure)
```

CTest запускает тесты лексера, парсера, семантического анализатора, интерпретатора и все сквозные тесты. При первой настройке тестовой сборки CMake скачивает GoogleTest.
