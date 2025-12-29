#include <iostream>        // cout, endl
#include <vector>          // vector
#include <random>          // mt19937, uniform_int_distribution
#include <chrono>          // измерение времени
#include <algorithm>       // is_sorted
#include <omp.h>           // OpenMP

using namespace std;

// Структура для reduction (min + индекс)
struct MinPair {           // Структура "минимум + позиция"
    int val;               // Значение
    int idx;               // Индекс
};

// Объявляем пользовательский reduction для выбора меньшего val
#pragma omp declare reduction( minpair : MinPair : \
    omp_out = (omp_in.val < omp_out.val ? omp_in : omp_out) ) \
    initializer(omp_priv = omp_orig)

// Генерация массива
static vector<int> make_random_array(int n) {                // Функция создаёт рандомный массив
    vector<int> a(n);                                       // Выделяем память под массив
    mt19937 rng(42);                                        // Генератор с фиксированным seed
    uniform_int_distribution<int> dist(-100000, 100000);    // Диапазон чисел
    for (int i = 0; i < n; i++) {                           // Идём по всем элементам
        a[i] = dist(rng);                                   // Заполняем случайным числом
    }
    return a;                                               // Возвращаем массив
}

// Последовательная сортировка выбором
static void selection_sort_sequential(vector<int>& a) {      // Обычный selection sort
    int n = (int)a.size();                                  // Размер массива
    for (int i = 0; i < n - 1; i++) {                        // Шагаем по позициям
        int bestIdx = i;                                    // Считаем, что минимум — в i
        for (int j = i + 1; j < n; j++) {                    // Ищем минимум справа
            if (a[j] < a[bestIdx]) bestIdx = j;              // Обновляем индекс минимума
        }
        if (bestIdx != i) swap(a[i], a[bestIdx]);            // Ставим минимум на позицию i
    }
}

// Параллельная сортировка выбором (OpenMP)
// Идея: на каждом шаге i параллельно ищем минимум в диапазоне [i..n-1]
static void selection_sort_parallel(vector<int>& a) {        // Параллельная версия
    int n = (int)a.size();                                  // Размер массива
    for (int i = 0; i < n - 1; i++) {                        // Внешний цикл остаётся последовательным
        MinPair best;                                       // Лучший кандидат минимума
        best.val = a[i];                                    // Стартуем с a[i]
        best.idx = i;                                       // Индекс i

        // Параллельно ищем минимум в правой части
#pragma omp parallel for reduction(minpair: best)
        for (int j = i + 1; j < n; j++) {                    // Каждый поток проверяет свой кусок
            if (a[j] < best.val) {                           // Если нашли меньше
                best.val = a[j];                             // Запоминаем значение
                best.idx = j;                                // Запоминаем индекс
            }
        }

        if (best.idx != i) swap(a[i], a[best.idx]);          // Один swap после параллельного поиска
    }
}

// Измерение времени для одной функции сортировки
template <typename Func>
static double measure_ms(Func f) {                           // Шаблон для измерения времени
    auto start = chrono::high_resolution_clock::now();        // Старт
    f();                                                     // Выполнение
    auto end = chrono::high_resolution_clock::now();          // Конец
    return chrono::duration<double, milli>(end - start).count(); // Время в мс
}

// TASK 3 (вызывается из main.cpp)
void task3() {

    cout << "\nTask 3: Selection Sort + OpenMP\n";     // Заголовок

    // Проверяем два размера
    const int sizes[2] = { 1000, 10000 };                     // Размеры массивов

    for (int s = 0; s < 2; s++) {                              // Перебор размеров
        int N = sizes[s];                                      // Текущий размер
        cout << "\nN = " << N << "\n";                          // Печать размера

        vector<int> base = make_random_array(N);               // Базовый (одинаковый) массив

        vector<int> a1 = base;                                 // Копия для последовательной сортировки
        vector<int> a2 = base;                                 // Копия для параллельной сортировки

        double t_seq = measure_ms([&]() {                      // Замер времени последовательной
            selection_sort_sequential(a1);                     // Сортировка
            });

        double t_par = measure_ms([&]() {                      // Замер времени параллельной
            selection_sort_parallel(a2);                       // Сортировка
            });

        bool ok1 = is_sorted(a1.begin(), a1.end());            // Проверка: отсортирован ли a1
        bool ok2 = is_sorted(a2.begin(), a2.end());            // Проверка: отсортирован ли a2
        bool same = (a1 == a2);                                // Проверка: одинаковый результат

        cout << "Sequential Selection Sort:\n";                // Подпись
        cout << "  time = " << t_seq << " ms\n";               // Время

        cout << "OpenMP Parallel Selection Sort:\n";           // Подпись
        cout << "  time = " << t_par << " ms\n";               // Время

        cout << "Correct (sorted): " << ((ok1 && ok2) ? "YES" : "NO") << "\n"; // Корректность
        cout << "Same result: " << (same ? "YES" : "NO") << "\n";              // Совпадение

        if (t_par > 0.0) {                                     // Чтобы не делить на 0
            cout << "Speedup (seq/par): " << (t_seq / t_par) << "x\n"; // Ускорение
        }
    }

}
