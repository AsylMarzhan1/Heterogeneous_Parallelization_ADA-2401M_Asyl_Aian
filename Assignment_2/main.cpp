#include <iostream>
#pragma once // Защита от многократного включения файла

void task2();// Эти функции реализуют логику каждого отдельного задания
void task3();

using namespace std;

int main() {
    setlocale(LC_ALL, "");
    int choice; // Переменная для хранения выбора
    //Цикл, позволяющий запускать задания многократно
    while (true) {
        cout << "\nВыберите задание для запуска:\n";
        cout << "1 - Task 2\n";
        cout << "2 - Task 3\n";
        cout << "0 - Выход\n";
        cout << "Ввод: ";
        cin >> choice;
        // В зависимости от выбора запускает нужное задание
        switch (choice) {
        case 2:
            task2();
            break;
        case 3:
            task3();
            break;
        case 0:
            cout << "Выход из программы.\n";
            return 0;
        default:
            cout << "Ошибка: введите число от 0 до 4\n";
        }
    }
}
