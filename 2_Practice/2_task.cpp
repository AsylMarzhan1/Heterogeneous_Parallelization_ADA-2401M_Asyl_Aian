#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
// Подключение OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
// Заполняет вектор случайными числами в диапазоне [lo, hi]
static void fillRandom(vector<int>& a, int lo = 1, int hi = 1000000) {
    static random_device rd; // Источник энтропии
    static mt19937 gen(rd()); // Генератор
    uniform_int_distribution<int> dist(lo, hi);
    for (auto& x : a) x = dist(gen); // Присваиваем каждому элементу случайное значение
}
// Измерение времени выполнения
template <class Func>
static long long timeMs(Func f) {
    auto t0 = chrono::high_resolution_clock::now();
    f();
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
}
// Последовательная сортировка пузырьком
static void bubbleSortSeq(vector<int>& a) {
    int n = (int)a.size(); // Размер массива
    for (int pass = 0; pass < n - 1; pass++) {
        bool swapped = false; // Флаг наличия обменов за проход
        for (int j = 0; j < n - 1 - pass; j++) {
            if (a[j] > a[j + 1]) { // Сравнение соседних элементов
                swap(a[j], a[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break; // Если обменов не было — массив уже отсортирован
    }
}
// Последовательная сортировка выбором
static void selectionSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i; // Индекс текущего минимального элемента
        for (int j = i + 1; j < n; j++)
            if (a[j] < a[minIdx]) minIdx = j; // Поиск минимального элемента в хвосте массива
        if (minIdx != i) swap(a[i], a[minIdx]); // Перестановка минимального элемента на позицию i
    }
}
// Последовательная сортировка вставками
static void insertionSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 1; i < n; i++) {
        int key = a[i]; // Элемент, который нужно вставить
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j]; // Сдвиг элементов вправо
            j--;
        }
        a[j + 1] = key; // Вставка элемента на найденную позицию
    }
}
// Параллельная пузырьковая сортировка
static void bubbleSortOmpOddEven(vector<int>& a) {
    int n = (int)a.size(); // Каждая фаза сравнивает независимые пары элементов
    for (int phase = 0; phase < n; phase++) {
        int start = phase % 2; // Чётная или нечётная фаза

#ifdef _OPENMP
#pragma omp parallel for // каждый поток обрабатывает свою пару элементов
#endif
        for (int i = start; i < n - 1; i += 2) {
            if (a[i] > a[i + 1]) // Сравнение независимых пар элементов
                swap(a[i], a[i + 1]);
        }
    }
}
// Параллельная сортировка выбором
// параллельно ищем минимум на хвосте, затем редукция по потокам вручную
static void selectionSortOmp(vector<int>& a) {
    int n = (int)a.size();

    for (int i = 0; i < n - 1; i++) {
        int globalMinVal = a[i]; // Текущее минимальное значение
        int globalMinIdx = i;

#ifdef _OPENMP
        int threads = omp_get_max_threads(); // Количество потоков
        vector<int> localMinVal(threads, globalMinVal);
        vector<int> localMinIdx(threads, globalMinIdx);

#pragma omp parallel
        {
            int tid = omp_get_thread_num(); // ID потока
            int lVal = globalMinVal;
            int lIdx = globalMinIdx;
            // Каждый поток ищет локальный минимум
#pragma omp for nowait
            for (int j = i + 1; j < n; j++) {
                if (a[j] < lVal) {
                    lVal = a[j];
                    lIdx = j;
                }
            }
            // Сохраняем локальные результаты
            localMinVal[tid] = lVal;
            localMinIdx[tid] = lIdx;
        }
        // Ручная редукция локальных минимумов
        for (int t = 0; t < threads; t++) {
            if (localMinVal[t] < globalMinVal) {
                globalMinVal = localMinVal[t];
                globalMinIdx = localMinIdx[t];
            }
        }
#else
        // Последовательный вариант (если OpenMP отключён)
        for (int j = i + 1; j < n; j++) {
            if (a[j] < globalMinVal) {
                globalMinVal = a[j];
                globalMinIdx = j;
            }
        }
#endif
        // Перестановка минимума
        if (globalMinIdx != i) swap(a[i], a[globalMinIdx]);
    }
}

// сортируем блоки вставками параллельно + потом параллельные merge-итерации
static void mergeRanges(vector<int>& a, vector<int>& tmp, int L, int M, int R) {
    int i = L, j = M, k = L;
    while (i < M && j < R) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < M) tmp[k++] = a[i++];
    while (j < R) tmp[k++] = a[j++]; 
    // Копирование обратно
    for (int t = L; t < R; t++) a[t] = tmp[t];
}

static void insertionSortOmpBlockMerge(vector<int>& a) {
    int n = (int)a.size();
    if (n <= 1) return;
    int T = 1;
#ifdef _OPENMP
    T = omp_get_max_threads(); // Число потоков
#endif

    int block = max(1024, n / T); // Размер блока
    int blocksCount = (n + block - 1) / block;
    // Сортировка блоков
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int b = 0; b < blocksCount; b++) {
        int L = b * block;
        int R = min(n, L + block);
        for (int i = L + 1; i < R; i++) {
            int key = a[i];
            int j = i - 1;
            while (j >= L && a[j] > key) {
                a[j + 1] = a[j];
                j--;
            }
            a[j + 1] = key;
        }
    }
    // Параллельное слияние блоков
    vector<int> tmp(n);
    for (int width = block; width < n; width *= 2) {
        int step = 2 * width;

#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int L = 0; L < n; L += step) {
            int M = min(n, L + width);
            int R = min(n, L + step);
            if (M < R) mergeRanges(a, tmp, L, M, R);
        }
    }
}

static void runBenchForSize(int n) {
    vector<int> base(n);
    fillRandom(base);// Генерация исходных данных
    auto testOne = [&](const char* name, void(*sortFn)(vector<int>&)) {
        vector<int> a = base; // Копия массива для сравнения
        long long ms = timeMs([&] { sortFn(a); });
        cout << "  " << name << ": " << ms << " ms\n";
        };
    cout << "\nN = " << n << "\n";
    testOne("Послд. Метод пузырьком", bubbleSortSeq);
    testOne("Послд. Метод выбором", selectionSortSeq);
    testOne("Послд. Метод вставкой", insertionSortSeq);
    testOne("Парал. Метод пузырьком", bubbleSortOmpOddEven);
    testOne("Парал. Метод выбором", selectionSortOmp);
    testOne("Парал. Метод вставкой", insertionSortOmpBlockMerge);
}
// Функция запуска  задачи
void run_task2() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
#ifdef _OPENMP
    cout << "\n[Task 2] Потоки = " << omp_get_max_threads() << "\n";
#else
    cout << "\n[Task 2] OpenMP NOT enabled (macros _OPENMP not defined).\n";
#endif

    vector<int> sizes = { 1000, 10000, 100000 };
    for (int n : sizes) runBenchForSize(n); // Запуск тестов для каждого размера
    cout << "\n";} 
