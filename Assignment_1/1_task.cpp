#include <iostream> // Ввод и вывод данных (cout, cin)
#include <cstdlib> // rand(), srand()
#include <ctime> // Работа со временем

using namespace std;
void task1() {
    const int SIZE = 50000;
    // Динамическое выделение памяти
    int* arr = new int[SIZE];
    srand((unsigned)time(nullptr));
    long long sum = 0;
    // Заполнение массива случайными числами от 1 до 100
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100 + 1;
        sum += arr[i];
    }
    // Вычисление среднего значения
    double average = (double)sum / SIZE;
    cout << "[Task 1]\n";
    cout << "Среднее значение элементов массива: " << average << endl;
    // Освобождение памяти
    delete[] arr;
}
