#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <random>
#include <algorithm>

#include "constants.h"

class Universe {
    private:
        std::vector<std::string> elements;

        void printTableCompact() const;
        void printTablePaged() const;

    protected:
        int depth;
        int maxMultiplicity;

    public:
        Universe();
        Universe(int depth, int maxMultiplicity);
        ~Universe();

        static std::vector<std::string> generateGrayCode(int n);

        int getDepth() const;
        int getMaxMultiplicity() const;
        const std::vector<std::string>& getElements() const;

        bool contains(const std::string& element) const;
        int size() const;

        void print() const;
        void printTable() const;
};
