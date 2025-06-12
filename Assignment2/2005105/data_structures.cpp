#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <cmath>
#include <tuple>
#include <algorithm>

using namespace std;

#define PI 2 * acos(0.0)

class Vector
{

public:
    double x, y, z;

    Vector(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    Vector operator+(const Vector &v) const
    {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector operator-(const Vector &v) const
    {
        return Vector(x - v.x, y - v.y, z - v.z);
    }

    Vector operator*(const double &d) const
    {
        return Vector(x * d, y * d, z * d);
    }

    double dot(const Vector &v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector cross(const Vector &v) const
    {
        return Vector(y * v.z - v.y * z, v.x * z - x * v.z, x * v.y - v.x * y);
    }

    Vector normalize() const
    {
        double length = sqrt(x * x + y * y + z * z);
        if (fabs(length) <= numeric_limits<double>::epsilon())
            throw invalid_argument("Vector magnitude is zero, which is incompatible for normalization");
        return Vector(x / length, y / length, z / length);
    }

    Vector rotate(const Vector &axis, double angle) const
    {
        double theta = angle * PI / 180;
        Vector k = axis.normalize();

        Vector v1 = *this * cos(theta);
        Vector v2 = k.cross(*this) * sin(theta);
        Vector v3 = k * k.dot(*this) * (1 - cos(theta));

        return v1 + v2 + v3;
    }

    friend istream &operator>>(istream &input_stream, Vector &v)
    {
        input_stream >> v.x >> v.y >> v.z;
        return input_stream;
    }
};

class Point
{
public:
    double x, y, z;
    Point(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

class Line
{
public:
    Point p0, p1;
    Vector direction;

    Line(Point p0, Point p1) : p0(p0), p1(p1)
    {
        direction = Vector(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
    }

    pair<bool, Point> get_intersection_point(const Line &line) const
    {
        Vector perp_vector = direction.cross(line.direction);

        double perp_vector_square = perp_vector.dot(perp_vector);
        // Parallel / Coincident
        if (fabs(perp_vector_square) <= numeric_limits<double>::epsilon())
            return make_pair(false, Point());

        Vector w = Vector(line.p0.x - p0.x, line.p0.y - p0.y, line.p0.z - p0.z);
        Vector u = w.cross(line.direction);

        double scalar_parameter = u.dot(perp_vector) / perp_vector_square;
        Point p = Point(p0.x + scalar_parameter * direction.x, p0.y + scalar_parameter * direction.y, p0.z + scalar_parameter * direction.z);
        return make_pair(true, p);
    }
};

class Matrix
{
public:
    int row, col;
    vector<vector<double>> elements;

    Matrix(int row, int col) : row(row), col(col), elements(row, vector<double>(col, 0)) {}

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
};

Matrix generateIdentityMatrix(int size)
{
    Matrix result(size, size);
    for (int i = 0; i < size; ++i)
        result.elements[i][i] = 1;
    return result;
}