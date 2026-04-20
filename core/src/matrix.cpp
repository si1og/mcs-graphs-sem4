#include "matrix.h"
#include <iomanip>
#include <algorithm>
#include <cmath>

Matrix::Matrix(int rows, int cols, double fill)
    : m_rows(rows), m_cols(cols)
    , m_data(rows, std::vector<double>(cols, fill))
{}

int Matrix::rows() const { return m_rows; }
int Matrix::cols() const { return m_cols; }

double& Matrix::operator()(int i, int j) { return m_data[i][j]; }
double Matrix::operator()(int i, int j) const { return m_data[i][j]; }

Matrix Matrix::m_multiply(const Matrix& other,
                           m_BinaryOp combine,
                           m_BinaryOp aggregate,
                           double identity) const {
    Matrix result(m_rows, other.m_cols, identity);

    // в не Шимбелловой матрице identity - нейтральный элемент по сложению (ноль)
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < other.m_cols; ++j) {
            for (int k = 0; k < m_cols; ++k) {
                if (m_data[i][k] != identity && other(k, j) != identity) {
                    double val = combine(m_data[i][k], other(k, j));
                    result(i, j) = aggregate(result(i, j), val);
                }
            }
        }
    }

    return result;
}

Matrix Matrix::multiply(const Matrix& other) const {
    return m_multiply(other,
        [](double a, double b) { return a * b; },
        [](double a, double b) { return a + b; },
        0);
}

Matrix Matrix::operator*(const Matrix& other) const {
    return multiply(other);
}

Matrix Matrix::shimbellMin(const Matrix& other) const {
    return m_multiply(other,
        [](double a, double b) { return a + b; },
        [](double a, double b) { return std::min(a, b); },
        std::numeric_limits<double>::infinity());
}

Matrix Matrix::shimbellMax(const Matrix& other) const {
    return m_multiply(other,
        [](double a, double b) { return a + b; },
        [](double a, double b) { return std::max(a, b); },
        -std::numeric_limits<double>::infinity());
}

void Matrix::print(PrintMode mode) const {
    const int cellWidth = (mode == PrintMode::Double) ? 9 : 5;
    const int labelWidth = 3;

    std::cout << std::setw(labelWidth) << "" << " |";
    for (int j = 0; j < m_cols; ++j) {
        std::cout << std::setw(cellWidth) << std::defaultfloat << j;
    }
    std::cout << "\n";

    std::cout << std::string(labelWidth + 1, '-') << "+"
              << std::string(cellWidth * m_cols, '-') << "\n";

    for (int i = 0; i < m_rows; ++i) {
        std::cout << std::setw(labelWidth) << std::defaultfloat << i << " |";
        for (int j = 0; j < m_cols; ++j) {
            double val = m_data[i][j];
            if (std::isinf(val)) {
                std::cout << std::setw(cellWidth) << (val > 0 ? "+inf" : "-inf");;
            } else if (mode == PrintMode::Double) {
                std::cout << std::setw(cellWidth) << std::fixed << std::setprecision(1) << val;
            } else {
                std::cout << std::setw(cellWidth) << std::defaultfloat << static_cast<int>(val);
            }
        }
        std::cout << "\n";
    }
}
