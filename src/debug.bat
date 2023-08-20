cls && clang++ uci.cpp -g -ggdb -Og -fsanitize-undefined-trap-on-error -fsanitize-trap=undefined -fsanitize=address -o main.exe && cls && main.exe
