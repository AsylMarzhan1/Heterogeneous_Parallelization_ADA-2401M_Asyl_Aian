#include <iostream>
#include <cstdlib>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace chrono;
// Заполнение массива случайными числами
static void fillRandom(int* arr, int n) {
    // фиксированный seed — чтобы сравнение было честным/повторяемым
    srand(12345);
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100 + 1; // 1..100
    }
}

static double averageSequential(const int* arr, int n) {
    long long sum = 0;
    // Проходим по массиву и суммируем элементы
    for (int i = 0; i < n; i++) {
        sum += arr[i];}
    // Возвращаем среднее арифметическое
    return (double)sum / n;}

static double averageParallelOMP(const int* arr, int n) {
    long long sum = 0;
#ifdef _OPENMP
// parallel for — распараллеливает цикл
// reduction(+:sum) — каждая нить имеет свою копию sum,
// которая затем безопасно складывается в общий результат
#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
#else
    // если OpenMP выключен — считаем последовательно
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
#endif

    return (double)sum / n;}
// Основная функция
void task4() {
    const int SIZE = 5'000'000;
    int* arr = new int[SIZE];
    fillRandom(arr, SIZE);
    cout << "[Task 4]\n";
    cout << "Среднее значение: последовательный vs OpenMP reduction\n";
    // Последовательное
    auto startSeq = high_resolution_clock::now();
    double avgSeq = averageSequential(arr, SIZE);
    auto endSeq = high_resolution_clock::now();
    auto seqMs = duration_cast<milliseconds>(endSeq - startSeq).count();
    // Параллельное
    auto startPar = high_resolution_clock::now();
    double avgPar = averageParallelOMP(arr, SIZE);
    auto endPar = high_resolution_clock::now();
    auto parMs = duration_cast<milliseconds>(endPar - startPar).count();
    cout << "\nРезультаты:\n";
    cout << "Послед -> avg: " << avgSeq << ", time: " << seqMs << " ms\n";
    cout << "Парал   -> avg: " << avgPar << ", time: " << parMs << " ms\n";
    // Проверка близости результатов(на всякий)
    double diff = avgSeq - avgPar;
    if (diff < 0) diff = -diff;
    // Проверка корректности результатов
    if (diff < 1e-9) {
        cout << "Проверка: OK (средние совпадают)\n";}
    else {
        cout << "Проверка: WARNING (разница = " << diff << ")\n";}
    // Расчет ускорения
    if (parMs > 0) {
        double speedup = (double)seqMs / (double)parMs;
        cout << "Ускорение (speedup): " << speedup << "x\n";}
    else {
        cout << "Ускорение: нельзя посчитать (слишком маленькое время)\n";}
    // Освобождение выделенной памяти
    delete[] arr;
}
