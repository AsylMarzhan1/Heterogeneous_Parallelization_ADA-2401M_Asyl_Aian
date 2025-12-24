#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <functional>
// Подключаем OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// Заполняет массив случайными целыми числами в диапазоне [lo, hi]
static void fillRandom(vector<int>& a, int lo = 1, int hi = 1000000) {
    static random_device rd; // Источник энтропии
    static mt19937 gen(rd()); // Генератор случайных чисел
    uniform_int_distribution<int> dist(lo, hi);
    for (auto& x : a) x = dist(gen);
}

// Измерение времени выполнения (в миллисекундах)
template <class Func>
static long long timeMs(Func f) {
    auto t0 = chrono::high_resolution_clock::now();
    f();
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(t1 - t0).count(); // Возвращаем длительность в миллисекундах
}

// Среднее время за несколько запусков
static long long averageTime(function<void()> f, int runs) {
    long long sum = 0;
    for (int i = 0; i < runs; i++)
        sum += timeMs(f);
    return sum / runs;
}
// Последовательная пузырьковая сортировка
static void bubbleSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int pass = 0; pass < n - 1; pass++) {
        bool swapped = false; // Флаг наличия обменов за проход
        for (int j = 0; j < n - 1 - pass; j++) {
            // Сравнение соседних элементов
            if (a[j] > a[j + 1]) {
                swap(a[j], a[j + 1]); // Обмен элементов
                swapped = true;
            }
        }
        // Если обменов не было, массив уже отсортирован
        if (!swapped) break;
    }
}
// Последовательная сортировка выбором
static void selectionSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i; // Индекс минимального элемента
        for (int j = i + 1; j < n; j++) // Поиск минимального элемента в неотсортированной части
            if (a[j] < a[minIdx]) minIdx = j;
        // Перестановка минимального элемента на позицию i
        swap(a[i], a[minIdx]);
    }
}
// Последовательная сортировка вставками
static void insertionSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 1; i < n; i++) {
        int key = a[i]; // Элемент для вставки
        int j = i - 1;
        // Сдвигаем элементы вправо, пока не найдём позицию
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        // Вставляем элемент на нужную позицию
        a[j + 1] = key;
    }
}

// Параллельная пузырьковая сортировка 
static void bubbleSortOmp(vector<int>& a) {
    int n = (int)a.size();
    // Каждая фаза сравнивает независимые пары элементов
    for (int phase = 0; phase < n; phase++) {
        int start = phase % 2; // Чётная или нечётная фаза
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int i = start; i < n - 1; i += 2) {
            // Сравнение независимых пар элементов
            if (a[i] > a[i + 1])
                swap(a[i], a[i + 1]);
        }
    }
}

// Параллельная сортировка выбором (поиск минимума на хвосте в параллельных потоках)
static void selectionSortOmp(vector<int>& a) {
    int n = (int)a.size();

    for (int i = 0; i < n - 1; i++) {
        int globalMinVal = a[i]; // Глобальный минимум
        int globalMinIdx = i;

#ifdef _OPENMP
        int threads = omp_get_max_threads(); // Количество потоков
        vector<int> localMinVal(threads, globalMinVal);
        vector<int> localMinIdx(threads, globalMinIdx);

#pragma omp parallel
        {
            int tid = omp_get_thread_num();
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
            // Сохраняем локальный результат
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
        for (int j = i + 1; j < n; j++) {
            if (a[j] < globalMinVal) {
                globalMinVal = a[j];
                globalMinIdx = j;
            }
        }
#endif
        // Перестановка найденного минимум
        swap(a[i], a[globalMinIdx]);
    }
}
// Параллельная сортировка вставками: блочная сортировка + слияние (merge)
static void mergeRanges(vector<int>& a, vector<int>& tmp, int L, int M, int R) {
    int i = L, j = M, k = L;
    while (i < M && j < R)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < M) tmp[k++] = a[i++];
    while (j < R) tmp[k++] = a[j++];
    // Копируем результат обратно в основной массив
    for (int t = L; t < R; t++) a[t] = tmp[t];
}

static void insertionSortOmp(vector<int>& a) {
    int n = (int)a.size();
    if (n <= 1) return;

    int T = 1;
#ifdef _OPENMP
    T = omp_get_max_threads(); // Количество потоков
#endif

    int block = max(1024, n / T); // Размер одного блока
    int blocks = (n + block - 1) / block; // Количество блоков
    // Сортировка каждого блока параллельно
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int b = 0; b < blocks; b++) {
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
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int L = 0; L < n; L += 2 * width) {
            int M = min(n, L + width);
            int R = min(n, L + 2 * width);
            if (M < R)
                mergeRanges(a, tmp, L, M, R);
        }
    }
}

struct Times {
    long long seqMs; // Время последовательной версии
    long long ompMs; // Время параллельной версии
};

static Times measureTimes(
    const vector<int>& base,
    void(*seqFn)(vector<int>&),
    void(*ompFn)(vector<int>&),
    int runs) {
    Times t{};
    // Измерение времени последовательной версии
    t.seqMs = averageTime([&] {
        vector<int> a = base;
        seqFn(a);
        }, runs);
    // Измерение времени параллельной версии
    t.ompMs = averageTime([&] {
        vector<int> a = base;
        ompFn(a);
        }, runs);
    return t;
}
void run_task3() {
#ifdef _OPENMP
    cout << "\n[Task 3] Потоки: " << omp_get_max_threads() << "\n";
#else
    cout << "\n[Задача 3] OpenMP НЕ включён (_OPENMP не определён).\n";
#endif

    vector<int> sizes = { 1000, 10000, 100000 };
    // Чтобы не ждать слишком долго на больших массивах
    auto runsFor = [](int n) {
        if (n <= 1000) return 5;
        if (n <= 10000) return 3;
        return 1;
        };
    for (int n : sizes) {
        vector<int> base(n);
        fillRandom(base); // Генерация исходного массива
        int runs = runsFor(n);
        // Измеряем время для всех алгоритмов
        auto tb = measureTimes(base, bubbleSortSeq, bubbleSortOmp, runs);
        auto ts = measureTimes(base, selectionSortSeq, selectionSortOmp, runs);
        auto ti = measureTimes(base, insertionSortSeq, insertionSortOmp, runs);
        //Вывод результатов
        cout << "Размер массива: " << n << " (усреднение: " << runs << " прог.)\n";
        cout << left << setw(18) << "Алгоритм"
            << right << setw(14) << "Seq (мс)"
            << right << setw(14) << "OMP (мс)" << "\n";
        auto print = [&](const string& name, const Times& t) {
            cout << left << setw(18) << name
                << right << setw(14) << t.seqMs
                << right << setw(14) << t.ompMs << "\n";
            };
        print("Метод пузырьком", tb);
        print("Метод выбором", ts);
        print("Метод вставкой", ti);
    }
    cout << "\n";
}
