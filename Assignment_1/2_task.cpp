#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono> // Измерение времени выполнения

using namespace std;
using namespace chrono;

void task2() {
    const int SIZE = 1'000'000;
    // Динамическое выделение памяти
    int* arr = new int[SIZE];
    srand((unsigned)time(nullptr));
    // Заполнение массива случайными числами от 1 до 1 000 000
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand();
    }
    cout << "[Task 2]\n";
    cout << "Поиск минимума и максимума (последовательно)\n";
    // Замер времени начала
    auto start = high_resolution_clock::now();
    int minVal = arr[0];
    int maxVal = arr[0];

    // Последовательный поиск min и max
    for (int i = 1; i < SIZE; i++) {
        if (arr[i] < minVal)
            minVal = arr[i];
        if (arr[i] > maxVal)
            maxVal = arr[i];
    }
    // Замер времени окончания
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Минимум: " << minVal << endl;
    cout << "Максимум: " << maxVal << endl;
    cout << "Время выполнения: " << duration.count() << " мс\n";
    // Освобождение памяти
    delete[] arr;
}
