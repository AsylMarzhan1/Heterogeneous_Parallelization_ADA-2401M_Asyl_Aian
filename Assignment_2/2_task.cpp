#include <iostream>        // Для ввода-вывода (cout)
#include <vector>          // Для использования std::vector
#include <random>          // Для генерации случайных чисел
#include <chrono>          // Для измерения времени выполнения
#include <omp.h>           // Для работы с OpenMP

using namespace std;

// Реализация задания 2
void task2() {

    const int N = 10000;   // Размер массива
    vector<int> arr(N);    // Создаём массив из 10 000 элементов

    // Генерация случайных чисел
    mt19937 rng(42);       // Генератор случайных чисел (фиксированный seed)
    uniform_int_distribution<int> dist(-100000, 100000); // Диапазон чисел

    for (int i = 0; i < N; i++) {
        arr[i] = dist(rng);    // Заполняем массив случайными числами
    }

    // Последовательный поиск min/max
    int min_seq = arr[0];  // Минимум (последовательно)
    int max_seq = arr[0];  // Максимум (последовательно)

    auto start_seq = chrono::high_resolution_clock::now(); // Старт таймера

    for (int i = 1; i < N; i++) {
        if (arr[i] < min_seq) min_seq = arr[i]; // Проверка минимума
        if (arr[i] > max_seq) max_seq = arr[i]; // Проверка максимума
    }

    auto end_seq = chrono::high_resolution_clock::now();   // Конец таймера
    double time_seq = chrono::duration<double, milli>(end_seq - start_seq).count();
    // Параллельный поиск min/max (OpenMP)

    int min_par = arr[0];  // Минимум (параллельно)
    int max_par = arr[0];  // Максимум (параллельно)

    auto start_par = chrono::high_resolution_clock::now(); // Старт таймера

#pragma omp parallel for reduction(min:min_par) reduction(max:max_par)
    for (int i = 0; i < N; i++) {
        if (arr[i] < min_par) min_par = arr[i]; // Каждый поток ищет минимум
        if (arr[i] > max_par) max_par = arr[i]; // Каждый поток ищет максимум
    }

    auto end_par = chrono::high_resolution_clock::now();   // Конец таймера
    double time_par = chrono::duration<double, milli>(end_par - start_par).count();

    // Вывод результатов
    cout << "\n Task 2: Min/Max + OpenMP \n";

    cout << "Последовательная версия:\n";
    cout << "  Min = " << min_seq << "\n";
    cout << "  Max = " << max_seq << "\n";
    cout << "  Time = " << time_seq << " ms\n\n";

    cout << "Параллельная версия (OpenMP):\n";
    cout << "  Min = " << min_par << "\n";
    cout << "  Max = " << max_par << "\n";
    cout << "  Time = " << time_par << " ms\n\n";

    // Проверка корректности
    if (min_seq == min_par && max_seq == max_par) {
        cout << "Результаты совпадают \n";
    }
    else {
        cout << "Ошибка: результаты различаются \n";
    }

}
