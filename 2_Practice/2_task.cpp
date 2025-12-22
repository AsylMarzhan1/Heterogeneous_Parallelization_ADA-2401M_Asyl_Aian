#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// -------------------- utils --------------------
static void fillRandom(vector<int>& a, int lo = 1, int hi = 1000000) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(lo, hi);
    for (auto& x : a) x = dist(gen);
}

static bool isSorted(const vector<int>& a) {
    for (size_t i = 1; i < a.size(); i++)
        if (a[i - 1] > a[i]) return false;
    return true;
}

template <class Func>
static long long timeMs(Func f) {
    auto t0 = chrono::high_resolution_clock::now();
    f();
    auto t1 = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
}

void bubbleSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int pass = 0; pass < n - 1; pass++) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - pass; j++) {
            if (a[j] > a[j + 1]) {
                swap(a[j], a[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void selectionSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++)
            if (a[j] < a[minIdx]) minIdx = j;
        if (minIdx != i) swap(a[i], a[minIdx]);
    }
}

void insertionSortSeq(vector<int>& a) {
    int n = (int)a.size();
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

void bubbleSortOmpOddEven(vector<int>& a) {
    int n = (int)a.size();
    for (int phase = 0; phase < n; phase++) {
        int start = phase % 2; // 0: (0,1)(2,3)... ; 1: (1,2)(3,4)...
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (int i = start; i < n - 1; i += 2) {
            if (a[i] > a[i + 1]) swap(a[i], a[i + 1]);
        }
    }
}

void selectionSortOmp(vector<int>& a) {
    int n = (int)a.size();

    for (int i = 0; i < n - 1; i++) {
        int globalMinVal = a[i];
        int globalMinIdx = i;

#ifdef _OPENMP
        int threads = omp_get_max_threads();
        vector<int> localMinVal(threads, globalMinVal);
        vector<int> localMinIdx(threads, globalMinIdx);

#pragma omp parallel
        {
            int tid = omp_get_thread_num();
            int lVal = globalMinVal;
            int lIdx = globalMinIdx;

#pragma omp for nowait
            for (int j = i + 1; j < n; j++) {
                if (a[j] < lVal) {
                    lVal = a[j];
                    lIdx = j;
                }
            }

            localMinVal[tid] = lVal;
            localMinIdx[tid] = lIdx;
        }

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

        if (globalMinIdx != i) swap(a[i], a[globalMinIdx]);
    }
}


static void mergeRanges(vector<int>& a, vector<int>& tmp, int L, int M, int R) {
    int i = L, j = M, k = L;
    while (i < M && j < R) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < M) tmp[k++] = a[i++];
    while (j < R) tmp[k++] = a[j++];
    for (int t = L; t < R; t++) a[t] = tmp[t];
}

void insertionSortOmpBlockMerge(vector<int>& a) {
    int n = (int)a.size();
    if (n <= 1) return;

    int T = 1;
#ifdef _OPENMP
    T = omp_get_max_threads();
#endif

    int block = max(1024, n / T);
    int blocksCount = (n + block - 1) / block;

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
    fillRandom(base);

    auto testOne = [&](const char* name, void(*sortFn)(vector<int>&)) {
        vector<int> a = base; 
        long long ms = timeMs([&] { sortFn(a); });
        cout << "  " << name << ": " << ms << " ms\n";
        };

    cout << "\nN = " << n << "\n";

    // Последовательные
    testOne("Seq Bubble", bubbleSortSeq);
    testOne("Seq Selection", selectionSortSeq);
    testOne("Seq Insertion", insertionSortSeq);

    // Параллельные (OpenMP)
    testOne("OMP Bubble", bubbleSortOmpOddEven);
    testOne("OMP Selection", selectionSortOmp);
    testOne("OMP Insertion", insertionSortOmpBlockMerge);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    vector<int> sizes = { 1000, 10000, 100000 };

    for (int n : sizes) {
        runBenchForSize(n);
    }
    return 0;
}
