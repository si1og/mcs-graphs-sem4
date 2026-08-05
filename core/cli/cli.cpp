#include "cli.h"


CLI::CLI() : m_vertexCount(0) {}

void CLI::m_generateGraph() {
    m_vertexCount = m_readInt(
        "Количество вершин (>=2): ",
        2,
        100
    );

    m_graph = std::make_unique<GeneratorGraph>(
        m_vertexCount
    );

    m_graph->generate();
}

void CLI::m_printHeader(const std::string& title) const {
    std::cout << "\n===== " << title << " =====\n";
}

void CLI::m_printInfo(const std::string& message) const {
    std::cout << "[*] " << message << "\n";
}

void CLI::m_printWarning(const std::string& message) const {
    std::cout << "[!] " << message << "\n";
}

void CLI::m_printError(const std::string& message) const {
    std::cout << "[x] " << message << "\n";
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

        m_printError("Некорректный ввод.");

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

    m_printInfo("Граф сгенерирован.");

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

    m_printInfo("Весовая матрица сгенерирована.");

    m_graph->printWeightMatrix();
}

void CLI::m_menuShimbell() const {
    m_printHeader("Алгоритм Шимбелла");

    if (!m_graph->isMatrixInit.weight) {
        m_printError("Весовая матрица не инициализирована.");
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
        m_printError("Весовая матрица не инициализирована.");
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

        std::cout
            << "Количество итераций: "
            << result.iterations
            << "\n";

        return;
    }

    std::string wayString;
    std::string weightsString;

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
            weightsString += ", ";
        }

        weightsString +=
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
        << "Веса дуг маршрута: ("
        << weightsString
        << "); sum: "
        << sum
        << "\n";

    std::cout
        << "Вектор расстояний: (";

    for (int i = 0; i < n; ++i) {
        if (i > 0) {
            std::cout << ", ";
        }

        if (result.T[i] ==
            std::numeric_limits<double>::infinity()) {

            std::cout << "inf";

        } else {

            std::cout
                << static_cast<int>(result.T[i]);
        }
    }

    std::cout << ")\n";

    std::cout
        << "Количество итераций: "
        << result.iterations
        << "\n";
}

void CLI::m_menuCapacityAndCostMatrices() {
    m_printHeader("Матрицы пропускных способностей и стоимостей");

    m_graph->generateCapacityAndCostMatrices();

    m_printInfo("Матрицы сгенерированы.");
    std::cout << "\n";

    m_graph->printCapacityMatrix();

    std::cout << "\n";

    m_graph->printCostMatrix();
}

void CLI::m_menuMaxFlow() {
    m_printHeader("Максимальный поток");

    if (!m_graph->isCapacityMatrixGenerated()) {
        m_printError("Сначала сгенерируйте матрицу пропускных способностей.");
        return;
    }

    int vertexCount = m_graph->getVertexCount();

    std::cout << "Источник:\n";
    int source = m_readInt("> ", 0, vertexCount - 1);

    std::cout << "Сток:\n";
    int sink = m_readInt("> ", 0, vertexCount - 1);

    if (source == sink) {
        m_printError("Источник и сток должны различаться.");
        return;
    }

    auto result = m_graph->fordFulkerson(source, sink);

    std::cout << "\nМаксимальный поток: "
              << result.maxFlow
              << "\n";

    std::cout << "Количество итераций: "
              << result.iterations
              << "\n\n";

    std::cout << "Матрица потоков:\n";

    result.flowMatrix.print();
}

void CLI::m_menuMinCostFlow() {
    m_printHeader("Поток минимальной стоимости");

    if (!m_graph->isCapacityMatrixGenerated()) {
        m_printError("Сначала сгенерируйте матрицу пропускных способностей.");
        return;
    }

    if (!m_graph->isCostMatrixGenerated()) {
        m_printError("Сначала сгенерируйте матрицу стоимостей.");
        return;
    }

    int vertexCount = m_graph->getVertexCount();

    std::cout << "Источник:\n";
    int source = m_readInt("> ", 0, vertexCount - 1);

    std::cout << "Сток:\n";
    int sink = m_readInt("> ", 0, vertexCount - 1);

    if (source == sink) {
        m_printError("Источник и сток должны различаться.");
        return;
    }

    auto maxFlowResult = m_graph->fordFulkerson(source, sink);

    int requiredFlow = (2 * maxFlowResult.maxFlow) / 3;

    if (requiredFlow <= 0 && maxFlowResult.maxFlow > 0) {
        requiredFlow = 1;
    }

    std::cout << "Максимальный поток: "
              << maxFlowResult.maxFlow
              << "\n";

    std::cout << "Заданный поток [2/3 * max]: "
              << requiredFlow
              << "\n\n";

    auto minCostResult = m_graph->minCostFlow(
        source,
        sink,
        requiredFlow
    );

    if (!minCostResult.success) {
        m_printError("Не удалось построить поток заданной величины.");
        std::cout << "Достигнутый поток: "
                  << minCostResult.achievedFlow
                  << "\n";
        std::cout << "Количество итераций: "
                << minCostResult.iterations
                << "\n\n";
        return;
    }

    std::cout << "Поток минимальной стоимости найден.\n";
    std::cout << "Стоимость потока: "
              << minCostResult.totalCost
              << "\n";

    std::cout << "Количество итераций: "
              << minCostResult.iterations
              << "\n\n";

    std::cout << "Матрица потоков минимальной стоимости:\n";
    minCostResult.flowMatrix.print();

    std::cout << "\nМатрица стоимостей минимального потока:\n";
    minCostResult.costFlowMatrix.print();
}

// lab4
void CLI::m_menuSpanningTreesCount() {
    m_printHeader("Число остовных деревьев");

    auto result = m_graph->countSpanningTreesKirchhoff();

    std::cout << "Матрица Кирхгофа:\n";
    result.kirchhoffMatrix.print();

    std::cout << "\nАлгебраическое дополнение A11:\n";
    result.cofactorMatrix.print();

    std::cout << "\nЧисло остовных деревьев: "
              << result.count
              << "\n";
}

void CLI::m_menuKruskalMinimumSpanningTree() {
    m_printHeader("Минимальный остов: алгоритм Краскала");

    if (!m_graph->isMatrixInit.weight) {
        m_printError("Сначала сгенерируйте весовую матрицу.");
        return;
    }

    auto result = m_graph->kruskalMinimumSpanningTree();

    if (!result.success) {
        m_printError("Не удалось построить остов: граф несвязный.");
        return;
    }

    std::cout << "Рёбра минимального остова:\n";

    for (const auto& edge : result.edges) {
        std::cout << edge.from
                  << " -- "
                  << edge.to
                  << " вес: "
                  << edge.weight
                  << "\n";
    }

    std::cout << "\nВес минимального остова: "
              << result.totalWeight
              << "\n\n";

    std::cout << "Матрица весов минимального остова:\n";
    result.spanningTreeMatrix.print();

    std::cout << "\nКод Прюфера: ";
    for (size_t i = 0; i < result.pruferCode.size(); ++i) {
        std::cout << result.pruferCode[i].vertex;

        if (i + 1 < result.pruferCode.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "\n";

    std::cout << "Веса кода Прюфера: ";
    for (size_t i = 0; i < result.pruferCode.size(); ++i) {
        std::cout << result.pruferCode[i].weight;

        if (i + 1 < result.pruferCode.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "\n\n";

    std::cout << "Матрица весов декодированного остова:\n";
    result.decodedTreeMatrix.print();

    std::cout << "\nПроверка декодирования: "
              << (result.pruferRoundTripSuccess
                  ? "остов совпадает"
                  : "остов не совпадает")
              << "\n";
}

void CLI::m_menuEdmondsBlossom() {
    m_printHeader("Алгоритм Эдмондса: максимальное независимое множество рёбер");

    std::cout << "Где искать:\n"
              << "  1 — исходный граф\n"
              << "  2 — минимальный остов\n";

    int target = m_readInt("> ", 1, 2);

    if (target == 2 && !m_graph->isMatrixInit.weight) {
        m_printError(
            "Сначала сгенерируйте весовую матрицу "
            "для построения минимального остова."
        );
        return;
    }

    auto result = (target == 1)
        ? m_graph->edmondsBlossomInOriginalGraph()
        : m_graph->edmondsBlossomInMinimumSpanningTree();

    if (!result.success) {
        m_printError("Не удалось построить множество рёбер.");
        return;
    }

    std::cout << "Размер множества: "
              << result.edges.size()
              << "\n";

    std::cout << "Рёбра:\n";

    if (result.edges.empty()) {
        std::cout << "  нет рёбер\n";
    } else {
        for (const auto& edge : result.edges) {
            std::cout << edge.from
                      << " -- "
                      << edge.to;

            if (result.hasWeights) {
                std::cout << " вес: "
                          << edge.weight;
            }

            std::cout << "\n";
        }
    }

    std::cout << "\nМатрица независимого множества рёбер:\n";
    result.edgeMatrix.print();
}

void CLI::m_menuCheckEulerianGraph() const {
    m_printHeader("Проверка, является ли неориентированный граф эйлеровым, построение эйлерова цикла");

    if (!m_graph->isMatrixInit.adjacency) {
        m_printError(
            "Сначала сгенерируйте матрицу смежности "
            "неориентированного графа."
        );
        return;
    }

    auto result = m_graph->checkIfEulerianGraph();

    if (!result.transformationCompleted) {
        m_printWarning(
            "Невозможно завершить преобразование, не удалив мост "
            "и не нарушив связность графа."
        );
        std::cout << "  1 — отменить преобразование\n"
                  << "  2 — разрешить разбиение графа на компоненты\n";

        const int choice = m_readInt("> ", 1, 2);

        if (choice == 1) {
            m_printInfo("Преобразование отменено.");
            return;
        }

        result = m_graph->checkIfEulerianGraph(true);
    }

    m_printInfo(
        std::string("Неориентированный граф ")
        + (result.isEulerian ? "является" : "не является")
        + " эйлеровым."
    );

    if (!result.resultIsConnected) {
        m_printWarning(
            "После преобразования все степени чётные, но граф разделён "
            "на компоненты. Единый эйлеров цикл построить нельзя."
        );
    }

    if (result.addedEdges.empty() && result.removedEdges.empty()) {
        m_printInfo("Изменения графа не требуются.");
    } else {
        std::cout << "\nИзменения графа:\n";

        if (!result.removedEdges.empty()) {
            std::cout << "  удалены ребра:\n";
            for (const auto& edge : result.removedEdges) {
                std::cout << edge.from
                          << " -- "
                          << edge.to
                          << "\n";
            }
        }

        if (!result.addedEdges.empty()) {
            std::cout << "  добавлены ребра:\n";
            for (const auto& edge : result.addedEdges) {
                std::cout << edge.from
                          << " -- "
                          << edge.to
                          << "\n";
            }
        }
    }

    std::cout << (result.resultIsConnected
        ? "\nМатрица смежности эйлерова графа:\n"
        : "\nМатрица графа с чётными степенями:\n");
    result.eulerianAdjacencyMatrix.print();

    if (!result.eulerianCycle.empty()) {
        std::cout << "\nЭйлеров цикл: ";

        for (size_t i = 0; i < result.eulerianCycle.size(); ++i) {
            std::cout << result.eulerianCycle[i];

            if (i + 1 < result.eulerianCycle.size()) {
                std::cout << " -> ";
            }
        }

        std::cout << "\n";
    }
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
        << "9. Сгенерировать матрицы пропускных способностей и стоимостей\n"
        << "10. Максимальный поток\n"
        << "11. Поток минимальной стоимости\n"
        << "12. Число остовных деревьев\n"
        << "13. Минимальный остов: алгоритм Краскала\n"
        << "14. Алгоритм Эдмондса: максимальное независимое множество рёбер\n"
        << "15. Проверка, является ли неориентированный граф эйлеровым, построение эйлерова цикла\n"
        << "0. Выход\n";
}

void CLI::run() {
    std::cout << "=== Теория графов ===\n";

    m_generateGraph();

    std::cout << "\n";
    m_printInfo("Граф сгенерирован автоматически.");

    m_graph->printAdjacencyMatrix();

    while (true) {
        m_printMenu();

        int choice = m_readInt("> ", 0, 15);

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

            case 9:
                m_menuCapacityAndCostMatrices();
                break;

            case 10:
                m_menuMaxFlow();
                break;

            case 11:
                m_menuMinCostFlow();
                break;

            case 12:
                m_menuSpanningTreesCount();
                break;

            case 13:
                m_menuKruskalMinimumSpanningTree();
                break;

            case 14:
                m_menuEdmondsBlossom();
                break;

            case 15:
                m_menuCheckEulerianGraph();
                break;
        }
    }

    std::cout << "Выход.\n";
}

int main() {
    CLI cli;
    cli.run();

    return 0;
}
