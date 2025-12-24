#include <iostream>

// Функции из других файлов
void run_task1();
void run_task2();
void run_task3();

int main() {
    setlocale(LC_ALL, "Russian");
    while (true) {
        std::cout << "1 - Задача 1\n";
        std::cout << "2 - Задача 2\n";
        std::cout << "3 - Задача 3\n";
        std::cout << "0 - Выход\n";
        std::cout << "Выбор: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(100000, '\n');
            std::cout << "Ошибка ввода. Повторите.\n";
            continue;
        }

        if (choice == 0) {
            std::cout << "Выход\n";
            break;
        }

        switch (choice) {
        case 1: run_task1(); break;
        case 2: run_task2(); break;
        case 3: run_task3(); break;
        default:
            std::cout << "Неверный выбор. Повторите.\n";
            break;
        }
    }
    return 0;
}
