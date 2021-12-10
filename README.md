# Сборка

Сборка происходит с помощью Cmake, итоговый бинарник будет называться lab03.

```
mkdir build
cd build
cmake ..
make
./lab03
```

Альтернативный вариант: сборка напрямую

```
g++ -std=c++17 -Wall -Wextra -O2 -I include src/cmd_parser.cpp src/elf32_parser.cpp src/registers.cpp src/main.cpp -o lab03
./lab03
```
