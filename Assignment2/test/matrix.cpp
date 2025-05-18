#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

using namespace std;

class Matrix
{
public:
    int row, col;
    vector<vector<double>> elements;

    Matrix(int row, int col) : row(row), col(col), elements(row, vector<double>(col, 0)) {}

    Matrix(vector<vector<double>> elements) : row(elements.size()), col(elements[0].size()), elements(elements) {}

    Matrix operator+(Matrix &other) const
    {
        if (row != other.row || col != other.col)
            throw invalid_argument("Matrix dimensions must match");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.elements[i][j] = elements[i][j] + other.elements[i][j];
        return result;
    }

    Matrix operator-(Matrix &other) const
    {
        if (row != other.row || col != other.col)
            throw invalid_argument("Matrix dimensions must match");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.elements[i][j] = elements[i][j] - other.elements[i][j];
        return result;
    }

    Matrix operator*(Matrix &other) const
    {
        if (col != other.row)
            throw invalid_argument("Incompatible dimensions for multiplication");
        Matrix result(row, other.col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < other.col; ++j)
                for (int k = 0; k < col; ++k)
                    result.elements[i][j] += elements[i][k] * other.elements[k][j];
        return result;
    }

    Matrix operator*(double scalar) const
    {
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.elements[i][j] = elements[i][j] * scalar;
        return result;
    }

    Matrix operator/(double scalar) const
    {
        if (fabs(scalar) <= numeric_limits<double>::epsilon())
            throw invalid_argument("Division by zero");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.elements[i][j] = elements[i][j] / scalar;
        return result;
    }

    Matrix transpose() const
    {
        Matrix result(col, row);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.elements[j][i] = elements[i][j];
        return result;
    }

    double determinant() const
    {
        if (row != col)
            throw invalid_argument("Matrix must be square");
        if (row == 1)
            return elements[0][0];
        double det = 0;
        for (int i = 0; i < col; ++i)
        {
            Matrix sub(row - 1, col - 1);
            for (int j = 1; j < row; ++j)
                for (int k = 0, colIdx = 0; k < col; ++k)
                    if (k != i)
                        sub.elements[j - 1][colIdx++] = elements[j][k];
            det += elements[0][i] * sub.determinant() * (i % 2 == 0 ? 1 : -1);
        }
        return det;
    }

    Matrix inverse() const
    {
        if (row != col)
            throw invalid_argument("Matrix must be square");
        double det = determinant();
        if (fabs(det) <= numeric_limits<double>::epsilon())
            throw invalid_argument("Matrix is singular");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                Matrix sub(row - 1, col - 1);
                for (int r = 0, rr = 0; r < row; ++r)
                {
                    if (r == i)
                        continue;
                    for (int c = 0, cc = 0; c < col; ++c)
                    {
                        if (c == j)
                            continue;
                        sub.elements[rr][cc++] = elements[r][c];
                    }
                    rr++;
                }
                double sign = ((i + j) % 2 == 0) ? 1 : -1;
                result.elements[i][j] = sign * sub.determinant() / det;
            }
        }
        return result.transpose();
    }

    friend ostream &operator<<(ostream &os, const Matrix &matrix)
    {
        for (int i = 0; i < matrix.row; ++i)
        {
            for (int j = 0; j < matrix.col; ++j)
                os << matrix.elements[i][j] << " ";
            if (i != matrix.row - 1)
                os << endl;
        }
        return os;
    }
};

Matrix generateIdentityMatrix(int size)
{
    Matrix result(size, size);
    for (int i = 0; i < size; ++i)
        result.elements[i][i] = 1;
    return result;
}