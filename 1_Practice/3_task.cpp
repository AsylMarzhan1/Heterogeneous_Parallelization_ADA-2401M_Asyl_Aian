#include <iostream>
#include <omp.h>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

//1)Заполнение массива случайными числами
void fill_random(int* arr, int N, int maxValue = 100) {
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % maxValue;  
    }
}
//2)Функция: среднее значение(последовательно)
double average_sequential(const int* arr, int N, double& time_ms) {
    long long sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        sum += arr[i];}
    auto end = chrono::high_resolution_clock::now();
    time_ms = chrono::duration<double, milli>(end - start).count();
    return static_cast<double>(sum) / N;
}
// 3) Функция: среднее значение (параллельно OpenMP) + reduction
double average_parallel_omp(const int* arr, int N, double& time_ms) {
    long long sum = 0;

    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += arr[i];}
    auto end = chrono::high_resolution_clock::now();

    time_ms = chrono::duration<double, milli>(end - start).count();
    return static_cast<double>(sum) / N;
}
int main() {
    setlocale(LC_ALL, "Russian");
    srand(static_cast<unsigned>(time(nullptr)));
    int N;
    cout << "Введите N (размер массива): ";
    cin >> N;
    if (N <= 0) {
        cout << "Ошибка: N должен быть больше 0.\n";
        return 1;}
    // 1) Динамический массив через указатель
    int* arr = new int[N];
    fill_random(arr, N, 99);
    // 2) Среднее последовательно (через функцию)
    double seq_time = 0.0;
    double avg_seq = average_sequential(arr, N, seq_time);
    // 3) Среднее параллельно OpenMP (через функцию)
    double par_time = 0.0;
    double avg_par = average_parallel_omp(arr, N, par_time);
    cout << "\nРезультаты:\n";
    cout << "Последовательно: среднее = " << avg_seq << ", time = " << seq_time << " ms\n";
    cout << "Параллельно:     среднее = " << avg_par << ", time = " << par_time << " ms\n";
    // 4) Освобождение памяти
    delete[] arr;
    return 0;
}
