#pragma once
#include "universe.h"

class Multiset : public Universe {
private:
    std::map<std::string, int> elements;

public:
    Multiset(const Universe& u);

    void fillManual(int size);
    void fillRandom();

    int getMultiplicity(const std::string& element) const;
    void setMultiplicity(const std::string& element, int m);

    Multiset unionWith(const Multiset& other) const;
    Multiset intersectionWith(const Multiset& other) const;
    Multiset differenceWith(const Multiset& other) const;
    Multiset symmetricDifferenceWith(const Multiset& other) const;
    Multiset complement() const;
    Multiset arithmeticSum(const Multiset& other) const;
    Multiset arithmeticDifference(const Multiset& other) const;
    Multiset arithmeticProduct(const Multiset& other) const;
    Multiset arithmeticDivision(const Multiset& other) const;

    int countNonZero() const;
    void printTable() const;
    void printTableCompact() const;
    void printTablePaged() const;

    bool isEmpty() const;
    bool operator!=(const Multiset& other) const;
    bool operator==(const Multiset& other) const;
};
