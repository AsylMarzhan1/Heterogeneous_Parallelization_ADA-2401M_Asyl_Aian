#include <iostream>
#include <vector>
#include <random>

// Сортировка пузырьком
void bubble_sort(std::vector<int>& a) {
    int n = a.size(); //Размер массива
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            // Сравнение соседних элементов
            if (a[j] > a[j + 1]) {
                // Обмен элементов местами
                std::swap(a[j], a[j + 1]);
            }
        }
    }
}

// Сортировка выбором
void selection_sort(std::vector<int>& a) {
    int n = a.size();
    for (int i = 0; i < n - 1; i++) {
        int min_index = i; // Индекс минимального элемента
        for (int j = i + 1; j < n; j++) {
            // Поиск минимального элемента
            if (a[j] < a[min_index]) {
                min_index = j;
            }
        }
        // Обмен текущего элемента с минимальным
        std::swap(a[i], a[min_index]);
    }
}

// Сортировка вставками
void insertion_sort(std::vector<int>& a) {
    int n = a.size();
    for (int i = 1; i < n; i++) {
        int key = a[i]; // Текущий элемент
        int j = i - 1;
        // Сдвигаем элементы вправо,
        // пока не найдём место для key
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        // Вставка элемента на нужную позицию
        a[j + 1] = key;
    }
}

// Функция запуска  задачи
void run_task1() {
    const int N = 10; // Размер массива
    // Генератор случайных чисел
    std::vector<int> original(N);
    std::mt19937 rng((unsigned)std::random_device{}());
    std::uniform_int_distribution<int> dist(1, 100);
    // Заполнение массива случайными числами
    for (int i = 0; i < N; i++)
        original[i] = dist(rng);
    std::cout << "\n[Task 1] Массив:\n"; // Вывод исходного массива
    for (int x : original) std::cout << x << " ";
    std::cout << "\n\n";
    std::vector<int> a;
    //Вывод результатов
    a = original;
    bubble_sort(a);
    std::cout << "Сортировка пузырьком:\n";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n\n";

    a = original;
    selection_sort(a);
    std::cout << "Сортировком выбором:\n";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n\n";

    a = original;
    insertion_sort(a);
    std::cout << "Сортировка вставкой:\n";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n\n";
}
