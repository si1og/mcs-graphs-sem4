#pragma once

#include <vector>
#include <iostream>
#include <limits>

class Matrix {
public:
    Matrix(int rows, int cols, double fill = 0);

    int rows() const;
    int cols() const;

    double& operator()(int i, int j);
    double operator()(int i, int j) const;

    Matrix operator*(const Matrix& other) const;
    Matrix multiply(const Matrix& other) const;
    Matrix shimbellMin(const Matrix& other) const;
    Matrix shimbellMax(const Matrix& other) const;

    enum class PrintMode { Integer, Double };
    void print(PrintMode mode = PrintMode::Integer) const;

private:
    using m_BinaryOp = double(*)(double, double);

    int m_rows, m_cols;
    std::vector<std::vector<double>> m_data;

    Matrix m_multiply(const Matrix& other,
                      m_BinaryOp combine,
                      m_BinaryOp aggregate,
                      double identity) const;
};
