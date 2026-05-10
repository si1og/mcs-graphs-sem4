#include "cli.h"


CLI::CLI() : m_vertexCount(0) {}

void CLI::m_generateGraph() {
    m_vertexCount = m_readInt(
        "Количество вершин (>=2): ",
        2,
        100
    );

    m_graph = std::make_unique<GeneratorGraph>(
        m_vertexCount,
        constants::WEIBULL_SCALE,
        constants::WEIBULL_SHAPE,
        constants::WEIBULL_SHIFT
    );

    m_graph->generate();
}

void CLI::m_printHeader(const std::string& title) const {
    std::cout << "\n===== " << title << " =====\n";
}

int CLI::m_readInt(const std::string& prompt,
                   int min,
                   int max) const {
    while (true) {
        std::cout << prompt;

        int value;

        if (std::cin >> value &&
            value >= min &&
            value <= max) {
            return value;
        }

        std::cout << "Некорректный ввод.\n";

        std::cin.clear();

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }
}

void CLI::m_printVector(const std::vector<int>& v) const {
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];

        if (i + 1 < v.size()) {
            std::cout << ", ";
        }
    }

    std::cout << "\n";
}

void CLI::m_menuGenerate() {
    m_printHeader("Генерация графа");

    m_generateGraph();

    std::cout << "Граф сгенерирован.\n";

    m_graph->printAdjacencyMatrix();
}

void CLI::m_menuEccentricities() {
    m_printHeader("Эксцентриситеты, центр, диаметр");

    m_graph->computeEccentricities();

    auto ecc = m_graph->getEccentricities();

    const int cellWidth = 4;

    std::cout << "Вершина:        ";

    for (size_t i = 0; i < ecc.size(); ++i) {
        std::cout << std::setw(cellWidth) << i;
    }

    std::cout << "\n";

    std::cout << "Эксцентриситет: ";

    for (double e : ecc) {
        std::cout << std::setw(cellWidth)
                  << static_cast<int>(e);
    }

    std::cout << "\n";

    std::cout << "Диаметр графа: "
              << m_graph->getDiameter()
              << "\n";

    std::cout << "Центральные вершины: ";
    m_printVector(m_graph->getCenterVertices());

    std::cout << "Диаметральные вершины: ";
    m_printVector(m_graph->getDiametralVertices());
}

void CLI::m_menuWeights() {
    m_printHeader("Весовая матрица");

    std::cout << "Режим весов:\n"
              << "  1 — положительные\n"
              << "  2 — отрицательные\n"
              << "  3 — смешанные\n";

    int mode = m_readInt("> ", 1, 3);

    WeightMode wm = WeightMode::Positive;

    switch (mode) {
        case 1:
            wm = WeightMode::Positive;
            break;

        case 2:
            wm = WeightMode::Negative;
            break;

        case 3:
            wm = WeightMode::Mixed;
            break;
    }

    m_graph->generateWeightMatrix(wm);

    std::cout << "Весовая матрица сгенерирована.\n";

    m_graph->printWeightMatrix();
}

void CLI::m_menuShimbell() const {
    m_printHeader("Алгоритм Шимбелла");

    if (!m_graph->isMatrixInit.weight) {
        std::cout << "Весовая матрица не инициализирована.\n";
        return;
    }

    int n = m_graph->getVertexCount();

    int steps = m_readInt(
        "Количество рёбер в пути [0, "
        + std::to_string(n - 1) + "]: ",
        0,
        n - 1
    );

    std::cout << "Искать:\n"
              << "  1 — минимальный путь\n"
              << "  2 — максимальный путь\n";

    int mode = m_readInt("> ", 1, 2);

    Matrix result =
        m_graph->shimbell(steps, mode == 1);

    result.print();
}

void CLI::m_menuRoutes() const {
    m_printHeader("Маршруты между вершинами");

    int n = m_graph->getVertexCount();

    int from = m_readInt(
        "Начальная вершина: ",
        0,
        n - 1
    );

    int to = m_readInt(
        "Конечная вершина: ",
        0,
        n - 1
    );

    bool exists = m_graph->hasRoute(from, to);

    std::cout << "Маршрут "
              << from
              << " -> "
              << to
              << ": "
              << (exists
                  ? "существует"
                  : "не существует")
              << "\n";

    if (exists) {
        std::cout << "Количество маршрутов: "
                  << m_graph->countRoutes(from, to)
                  << "\n";
    }
}

void CLI::m_menuPrintMatrices() const {
    m_printHeader("Текущие матрицы");

    m_graph->printAdjacencyMatrix();

    std::cout << "\n";

    m_graph->printWeightMatrix();
}

void CLI::m_menuFindArticulationPoints() const {
    m_printHeader("Точки сочленения");

    auto result =
        m_graph->findArticulationPoints();

    if (result.points.empty()) {
        std::cout << "Точки сочленения не найдены.\n";
    } else {
        std::cout << "Точки сочленения: ";

        for (int v : result.points) {
            std::cout << v << " ";
        }

        std::cout << "\n";
    }

    std::cout << "Количество итераций: "
              << result.iterations
              << "\n";
}

void CLI::m_runDijkstraNegative() {
    m_printHeader(
        "Алгоритм Дейкстры для отрицательных весов"
    );

    if (!m_graph->isMatrixInit.weight) {
        std::cout << "Весовая матрица не инициализирована.\n";
        return;
    }

    int n = m_graph->getVertexCount();

    int s = m_readInt(
        "Начальная вершина: ",
        0,
        n - 1
    );

    int t = m_readInt(
        "Конечная вершина: ",
        0,
        n - 1
    );

    auto result =
        m_graph->dijkstraNegative(s, t);

    if (!result.hasPath) {
        std::cout << "Путь не найден.\n";
        return;
    }

    std::string wayString;
    std::string distanceString;

    double sum = 0;

    for (size_t i = 0; i < result.path.size(); ++i) {
        if (i > 0) {
            wayString += ", ";
        }

        wayString +=
            std::to_string(result.path[i]);
    }

    for (size_t i = 1; i < result.path.size(); ++i) {
        int from = result.path[i - 1];
        int to = result.path[i];

        double weight =
            m_graph->getWeightMatrix()(from, to);

        if (i > 1) {
            distanceString += ", ";
        }

        distanceString +=
            std::to_string(static_cast<int>(weight));

        sum += weight;
    }

    std::cout
        << "Кратчайший маршрут из "
        << s
        << " в "
        << t
        << " : ("
        << wayString
        << ")\n";

    std::cout
        << "Вектор расстояний: ("
        << distanceString
        << "); sum: "
        << sum
        << "\n";

    std::cout
        << "Количество итераций: "
        << result.iterations
        << "\n";
}

void CLI::m_printMenu() const {
    std::cout
        << "\n----- Меню -----\n"
        << "1. Сгенерировать граф\n"
        << "2. Эксцентриситеты, центр, диаметр\n"
        << "3. Сгенерировать весовую матрицу\n"
        << "4. Алгоритм Шимбелла\n"
        << "5. Маршруты между вершинами\n"
        << "6. Показать текущие матрицы\n"
        << "7. Найти точки сочленения\n"
        << "8. Алгоритм Дейкстры для отрицательных весов\n"
        << "0. Выход\n";
}

void CLI::run() {
    std::cout << "=== Теория графов ===\n";

    m_generateGraph();

    std::cout
        << "\nГраф сгенерирован автоматически.\n";

    m_graph->printAdjacencyMatrix();

    while (true) {
        m_printMenu();

        int choice = m_readInt("> ", 0, 8);

        if (choice == 0) {
            break;
        }

        switch (choice) {
            case 1:
                m_menuGenerate();
                break;

            case 2:
                m_menuEccentricities();
                break;

            case 3:
                m_menuWeights();
                break;

            case 4:
                m_menuShimbell();
                break;

            case 5:
                m_menuRoutes();
                break;

            case 6:
                m_menuPrintMatrices();
                break;

            case 7:
                m_menuFindArticulationPoints();
                break;

            case 8:
                m_runDijkstraNegative();
                break;
        }
    }

    std::cout << "Выход.\n";
}
