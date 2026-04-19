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
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

void printVector(const std::vector<double>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << std::fixed << std::setprecision(1) << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

// ============================================================
// пункты меню
// ============================================================

void menuGenerate(GeneratorGraph& graph) {
    printHeader("п.1. Генерация графа");
    graph.generate();
    std::cout << "Граф сгенерирован.\n";
    graph.printAdjacencyMatrix();
}

void menuEccentricities(GeneratorGraph& graph) {
    printHeader("п.2. Эксцентриситеты, центр, диаметр");
    graph.computeEccentricities();

    std::cout << "Эксцентриситеты вершин: ";
    printVector(graph.getEccentricities());

    std::cout << "Диаметр графа: " << graph.getDiameter() << "\n";

    std::cout << "Центральные вершины: ";
    printVector(graph.getCenterVertices());

    std::cout << "Диаметральные вершины: ";
    printVector(graph.getDiametralVertices());
}

void menuWeights(GeneratorGraph& graph) {
    printHeader("п.3. Весовая матрица");
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
    printHeader("п.3. Алгоритм Шимбелла");
    int n = graph.getVertexCount();
    int steps = readInt("Количество рёбер в пути [1, " + std::to_string(n - 1) + "]: ",
                        1, n - 1);
    std::cout << "Искать:\n  1 — минимальный путь\n  2 — максимальный путь\n";
    int mode = readInt("> ", 1, 2);

    Matrix result = graph.shimbell(steps, mode == 1);
    std::cout << "Матрица " << (mode == 1 ? "минимальных" : "максимальных")
              << " путей длины " << steps << ":\n";
    result.print();
}

void menuRoutes(const GeneratorGraph& graph) {
    printHeader("п.4. Маршруты между вершинами");
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

// ============================================================
// главное меню
// ============================================================

void printMenu() {
    std::cout << "\n----- Меню -----\n"
              << "1. Сгенерировать граф\n"
              << "2. Эксцентриситеты, центр, диаметр\n"
              << "3. Сгенерировать весовую матрицу\n"
              << "4. Алгоритм Шимбелла\n"
              << "5. Маршруты между вершинами\n"
              << "6. Показать текущие матрицы\n"
              << "0. Выход\n";
}

} // namespace

int main() {
    std::cout << "=== Лабораторная работа №1 ===\n"
              << "Генерация ориентированного ациклического графа\n"
              << "(смещённое распределение Вейбулла-Гнеденко)\n\n";

    int n = readInt("Количество вершин [2, 100]: ", 2, 100);

    std::cout << "\nПараметры распределения Вейбулла-Гнеденко:\n";
    double a  = readDouble("  параметр масштаба a  (a > 0): ");
    double c  = readDouble("  параметр формы    c  (c > 0): ");
    double y0 = readDouble("  параметр сдвига   y0        : ");

    GeneratorGraph graph(n, a, c, y0);

    // сразу генерируем граф, чтобы остальные пункты были доступны
    graph.generate();
    std::cout << "\nГраф сгенерирован автоматически.\n";
    graph.printAdjacencyMatrix();

    while (true) {
        printMenu();
        int choice = readInt("> ", 0, 6);
        if (choice == 0) break;

        switch (choice) {
            case 1: menuGenerate(graph);        break;
            case 2: menuEccentricities(graph);  break;
            case 3: menuWeights(graph);         break;
            case 4: menuShimbell(graph);        break;
            case 5: menuRoutes(graph);          break;
            case 6: menuPrintMatrices(graph);   break;
        }
    }

    std::cout << "Выход.\n";
    return 0;
}
