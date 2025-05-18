#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <limits>
#include <limits>

using namespace std;

#define PI 2 * acos(0.0)

class Vector
{

public:
    double x, y, z;

    Vector(double x, double y, double z) : x(x), y(y), z(z) {}

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

    Vector operator/(const double &d) const
    {
        if (fabs(d) <= numeric_limits<double>::epsilon())
            throw invalid_argument("Division by zero");
        return Vector(x / d, y / d, z / d);
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
            throw invalid_argument("vector magnitude is 0, which is incompatible for normalization");
        return Vector(x / length, y / length, z / length);
    }

    Vector rotate(const Vector &axis, double angle) const
    {
        // Rodrigues' Rotation Formula
        double theta = angle * PI / 180;
        Vector k = axis.normalize();
        Vector v1 = *this * cos(theta);
        Vector v2 = k.cross(*this) * sin(theta);
        Vector v3 = k * k.dot(*this) * (1 - cos(theta));
        return v1 + v2 + v3;
    }

    bool check_normalized() const
    {
        double length = sqrt(x * x + y * y + z * z);
        return fabs(length - 1) <= numeric_limits<double>::epsilon();
    }

    bool check_orthogonal(const Vector &v) const
    {
        return fabs(this->dot(v)) <= numeric_limits<double>::epsilon();
    }

    double norm() const { return sqrt(x * x + y * y + z * z); }

    friend istream &operator>>(istream &is, Vector &v)
    {
        is >> v.x >> v.y >> v.z;
        return is;
    }

    friend ostream &operator<<(ostream &os, const Vector &v)
    {
        // os << v.x << " " << v.y << " " << v.z;
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};
