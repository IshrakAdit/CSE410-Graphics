#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>
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
};