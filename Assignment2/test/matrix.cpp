#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

using namespace std;

class Matrix
{
public:
    int row, col;
    vector<vector<double>> data;

    Matrix(int row, int col) : row(row), col(col), data(row, vector<double>(col, 0)) {}

    Matrix(vector<vector<double>> data) : row(data.size()), col(data[0].size()), data(data) {}

    Matrix operator+(Matrix &other) const
    {
        if (row != other.row || col != other.col)
            throw invalid_argument("Matrix dimensions must match");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.data[i][j] = data[i][j] + other.data[i][j];
        return result;
    }

    Matrix operator-(Matrix &other) const
    {
        if (row != other.row || col != other.col)
            throw invalid_argument("Matrix dimensions must match");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.data[i][j] = data[i][j] - other.data[i][j];
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
                    result.data[i][j] += data[i][k] * other.data[k][j];
        return result;
    }

    Matrix operator*(double scalar) const
    {
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.data[i][j] = data[i][j] * scalar;
        return result;
    }

    Matrix operator/(double scalar) const
    {
        if (fabs(scalar) <= numeric_limits<double>::epsilon())
            throw invalid_argument("Division by zero");
        Matrix result(row, col);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.data[i][j] = data[i][j] / scalar;
        return result;
    }

    Matrix identity(int size)
    {
        Matrix result(size, size);
        for (int i = 0; i < size; ++i)
            result.data[i][i] = 1;
        return result;
    }

    Matrix transpose() const
    {
        Matrix result(col, row);
        for (int i = 0; i < row; ++i)
            for (int j = 0; j < col; ++j)
                result.data[j][i] = data[i][j];
        return result;
    }

    double determinant() const
    {
        if (row != col)
            throw invalid_argument("Matrix must be square");
        if (row == 1)
            return data[0][0];
        double det = 0;
        for (int i = 0; i < col; ++i)
        {
            Matrix sub(row - 1, col - 1);
            for (int j = 1; j < row; ++j)
                for (int k = 0, colIdx = 0; k < col; ++k)
                    if (k != i)
                        sub.data[j - 1][colIdx++] = data[j][k];
            det += data[0][i] * sub.determinant() * (i % 2 == 0 ? 1 : -1);
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
                        sub.data[rr][cc++] = data[r][c];
                    }
                    rr++;
                }
                double sign = ((i + j) % 2 == 0) ? 1 : -1;
                result.data[i][j] = sign * sub.determinant() / det;
            }
        }
        return result.transpose();
    }

    friend ostream &operator<<(ostream &os, const Matrix &matrix)
    {
        for (int i = 0; i < matrix.row; ++i)
        {
            for (int j = 0; j < matrix.col; ++j)
                os << matrix.data[i][j] << " ";
            if (i != matrix.row - 1)
                os << endl;
        }
        return os;
    }
};