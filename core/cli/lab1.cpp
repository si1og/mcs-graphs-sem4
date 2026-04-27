#include "../src/lab1.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

namespace {

void printHeader(const std::string& title) {
    std::cout << "\n===== " << title << " =====\n";
}

int readInt(const std::string& prompt, int min, int max) {
    while (true) {
        std::cout << prompt;
        int value;
        if (std::cin >> value && value >= min && value <= max) {
            return value;
        }
        std::cout << "Некорректный ввод. Ожидается целое число в диапазоне ["
                  << min << ", " << max << "].\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

double readDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        double value;
        if (std::cin >> value) {
            return value;
        }
        std::cout << "Некорректный ввод. Ожидается число.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void printVector(const std::vector<int>& v) {
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "\n";
}

void printVector(const std::vector<double>& v) {
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << std::fixed << std::setprecision(1) << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "\n";
}

void printVectorDoubleToInt(const std::vector<double>& v) {
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << std::fixed << std::setprecision(1) << static_cast<int>(v[i]);
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "\n";
}

void menuGenerate(GeneratorGraph& graph) {
    printHeader("Генерация графа");
    graph.generate();
    std::cout << "Граф сгенерирован.\n";
    graph.printAdjacencyMatrix();

}

void menuEccentricities(GeneratorGraph& graph) {
    printHeader("Эксцентриситеты, центр, диаметр");
    graph.computeEccentricities();

    auto ecc = graph.getEccentricities();
    int n = ecc.size();
    const int cellWidth = 4;

    std::cout << "Вершина:        ";
    for (int i = 0; i < n; ++i) {
        std::cout << std::setw(cellWidth) << i;
    }
    std::cout << "\n";

    std::cout << "Эксцентриситет: ";
    for (int i = 0; i < n; ++i) {
        std::cout << std::setw(cellWidth) << static_cast<int>(ecc[i]);
    }
    std::cout << "\n";

    std::cout << "Диаметр графа: " << graph.getDiameter() << "\n";

    std::cout << "Центральные вершины: ";
    printVector(graph.getCenterVertices());

    std::cout << "Диаметральные вершины: ";
    printVector(graph.getDiametralVertices());
}

void menuWeights(GeneratorGraph& graph) {
    printHeader("Весовая матрица");
    std::cout << "Режим весов:\n"
              << "  1 — положительные\n"
              << "  2 — отрицательные\n"
              << "  3 — смешанные\n";
    int mode = readInt("> ", 1, 3);

    WeightMode wm = WeightMode::Positive;
    switch (mode) {
        case 1: wm = WeightMode::Positive; break;
        case 2: wm = WeightMode::Negative; break;
        case 3: wm = WeightMode::Mixed;    break;
    }

    graph.generateWeightMatrix(wm);
    std::cout << "Весовая матрица сгенерирована.\n";
    graph.printWeightMatrix();
}

void menuShimbell(const GeneratorGraph& graph) {
    printHeader("Алгоритм Шимбелла");
    if (!graph.isMatrixInit.weight) {
        std::cout << "Весовая матрица не инициализирована.\n";
        return;
    }

    int n = graph.getVertexCount();
    int steps = readInt("Количество рёбер в пути [0, " + std::to_string(n) + "]: ",
                        0, n);
    std::cout << "Искать:\n  1 — минимальный путь\n  2 — максимальный путь\n";
    int mode = readInt("> ", 1, 2);

    Matrix result = graph.shimbell(steps, mode == 1);
    std::cout << "Матрица " << (mode == 1 ? "минимальных" : "максимальных")
              << " путей длины " << steps << ":\n";
    result.print();
}

void menuRoutes(const GeneratorGraph& graph) {
    printHeader("Маршруты между вершинами");
    int n = graph.getVertexCount();
    int from = readInt("Начальная вершина [0, " + std::to_string(n - 1) + "]: ",
                       0, n - 1);
    int to = readInt("Конечная вершина  [0, " + std::to_string(n - 1) + "]: ",
                     0, n - 1);

    bool exists = graph.hasRoute(from, to);
    std::cout << "Маршрут " << from << " -> " << to << ": "
              << (exists ? "существует" : "не существует") << "\n";

    if (exists) {
        int count = graph.countRoutes(from, to);
        std::cout << "Количество различных маршрутов: " << count << "\n";
    }
}

void menuPrintMatrices(const GeneratorGraph& graph) {
    printHeader("Текущие матрицы");
    graph.printAdjacencyMatrix();
    std::cout << "\n";
    graph.printWeightMatrix();
}

void printMenu() {
    std::cout << "\n----- Меню -----\n"
              << "1. Сгенерировать граф\n"
              << "2. Эксцентриситеты, центр, диаметр\n"
              << "3. Сгенерировать весовую матрицу\n"
              << "4. Алгоритм Шимбелла\n"
              << "5. Маршруты между вершинами\n"
              << "6. Показать текущие матрицы\n"
              << "7. Тест распределения степеней\n"
              << "0. Выход\n";
}

}

int main() {
    std::cout << "=== Лабораторная работа №1 ===\n";

    int n = readInt("Количество вершин (>=2): ", 2, 100);

    GeneratorGraph graph(n, constants::WEIBULL_SCALE, constants::WEIBULL_SHAPE, constants::WEIBULL_SHIFT);

    graph.generate();
    std::cout << "\nГраф сгенерирован автоматически.\n";
    graph.printAdjacencyMatrix();

    while (true) {
        printMenu();
        int choice = readInt("> ", 0, 7);
        if (choice == 0) break;

        switch (choice) {
            case 1: menuGenerate(graph);        break;
            case 2: menuEccentricities(graph);  break;
            case 3: menuWeights(graph);         break;
            case 4: menuShimbell(graph);        break;
            case 5: menuRoutes(graph);          break;
            case 6: menuPrintMatrices(graph);   break;
            case 7: graph.testDistribution();   break;
        }
    }

    std::cout << "Выход.\n";
    return 0;
}
