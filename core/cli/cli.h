#pragma once

#include "../src/generator_graph.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

class CLI {
public:
    explicit CLI();
    void run();

private:
    int m_vertexCount;
    std::unique_ptr<GeneratorGraph> m_graph;

    void m_generateGraph();
    void m_printHeader(const std::string& title) const;
    int m_readInt(const std::string& prompt,
                  int min,
                  int max) const;
    void m_printVector(const std::vector<int>& v) const;
    void m_printMenu() const;

    void m_menuGenerate();
    void m_menuEccentricities();
    void m_menuWeights();
    void m_menuShimbell() const;
    void m_menuRoutes() const;
    void m_menuPrintMatrices() const;

    void m_menuFindArticulationPoints() const;
    void m_runDijkstraNegative();
};
