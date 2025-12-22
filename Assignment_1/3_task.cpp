#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <limits>  // numeric_limits для инициализации min/max

#ifdef _OPENMP
#include <omp.h> // Подключение OpenMP
#endif

using namespace std;
using namespace chrono;

// Функция заполнения массива случайными числами
static void fillRandom(int* arr, int n) {
    // фиксируем seed, чтобы последовательный и параллельный запуск сравнивались честно
    srand(12345);
    for (int i = 0; i < n; i++) {
        arr[i] = rand(); // 0..RAND_MAX
    }
}

// Последовательный поиск min/max
static void minmaxSequential(const int* arr, int n, int& outMin, int& outMax) {
    int mn = arr[0];
    int mx = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < mn) mn = arr[i];
        if (arr[i] > mx) mx = arr[i];
    }
    // Возвращаем результат через ссылки
    outMin = mn;
    outMax = mx;
}

// Параллельный поиск min/max (OpenMP)
static void minmaxParallelOMP(const int* arr, int n, int& outMin, int& outMax) {
#ifdef _OPENMP
    // Глобальные min и max для всех потоков
    int globalMin = numeric_limits<int>::max();
    int globalMax = numeric_limits<int>::min();
    // Создаем параллельную область
#pragma omp parallel
    {
        int localMin = numeric_limits<int>::max();
        int localMax = numeric_limits<int>::min();
    // Распределяем цикл по потокам
#pragma omp for nowait
        for (int i = 0; i < n; i++) {
            if (arr[i] < localMin) localMin = arr[i];
            if (arr[i] > localMax) localMax = arr[i];
        }

        // объединяем локальные результаты в глобальные
#pragma omp critical
        {
            if (localMin < globalMin) globalMin = localMin;
            if (localMax > globalMax) globalMax = localMax;
        }
    }
    // Возвращаем результат
    outMin = globalMin;
    outMax = globalMax;
#else
    // Если OpenMP не поддерживается, используем последовательный алгоритм
    minmaxSequential(arr, n, outMin, outMax);
#endif
}
// Основная функция
void task3() {
    const int SIZE = 1'000'000;
    int* arr = new int[SIZE];
    fillRandom(arr, SIZE);
    cout << "[Task 3]\n";
    cout << "Сравнение последовательного и параллельного (OpenMP) поиска min/max\n";

    // Последовательное измерени
    int seqMin = 0, seqMax = 0;
    auto startSeq = high_resolution_clock::now();
    minmaxSequential(arr, SIZE, seqMin, seqMax);
    auto endSeq = high_resolution_clock::now();
    auto seqMs = duration_cast<milliseconds>(endSeq - startSeq).count();

    // Параллельное измерение
    int parMin = 0, parMax = 0;
    auto startPar = high_resolution_clock::now();
    minmaxParallelOMP(arr, SIZE, parMin, parMax);
    auto endPar = high_resolution_clock::now();
    auto parMs = duration_cast<milliseconds>(endPar - startPar).count();
    cout << "\nРезультаты:\n";
    cout << "Послед -> min: " << seqMin << ", max: " << seqMax
        << ", time: " << seqMs << " ms\n";
    cout << "Парал   -> min: " << parMin << ", max: " << parMax
        << ", time: " << parMs << " ms\n";
    // Проверка корректности
    if (seqMin == parMin && seqMax == parMax) {
        cout << "Проверка: OK (результаты совпадают)\n";}
    else {
        cout << "Проверка: ERROR (результаты НЕ совпадают)\n";}
    // Расчет ускорения
    if (parMs > 0) {
        double speedup = (double)seqMs / (double)parMs;
        cout << "Ускорение (speedup): " << speedup << "x\n";}
    else {
        cout << "Ускорение: нельзя посчитать (слишком маленькое время)\n";}
    // Освобождение выделенной памяти
    delete[] arr;
}
