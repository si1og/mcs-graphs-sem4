#pragma once

#include "../src/generator_graph.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>


class CLI {
    private:
        void m_printHeader(const std::string& title);


    public:
        CLI();
        void run();


}
