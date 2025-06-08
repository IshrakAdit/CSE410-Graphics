#include <iostream>
#include <cmath>
#include <limits>
#include <tuple>
#include "vector.cpp"

using namespace std;

class Point
{
public:
    double x, y, z;

    Point(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    double distance(const Point &p) { return sqrt(pow(x - p.x, 2) + pow(y - p.y, 2) + pow(z - p.z, 2)); }

    friend ostream &operator<<(ostream &os, const Point &p)
    {
        os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
        return os;
    }
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

    Line(Point p0, Vector dir) : p0(p0), direction(dir)
    {
        p1 = Point(p0.x + dir.x, p0.y + dir.y, p0.z + dir.z);
    }

    pair<bool, Point> Line::get_intersection_point(const Line &l) const
    {
        Vector vec = direction.cross(l.direction);

        double res = vec.dot(vec);
        if (fabs(res) <= numeric_limits<double>::epsilon())
        {
            return make_pair(false, Point());
        }

        Vector w = Vector(l.p0.x - p0.x, l.p0.y - p0.y, l.p0.z - p0.z);
        Vector u = w.cross(l.direction);
        double t = u.dot(vec) / res;

        Point p = Point(p0.x + t * direction.x, p0.y + t * direction.y, p0.z + t * direction.z);
        return make_pair(true, p);
    }

    double get_distance_from_point(const Point &p) const
    {
        Vector b = Vector(p.x - p0.x, p.y - p0.y, p.z - p0.z);
        Vector c = direction.cross(b);
        return c.norm() / direction.norm();
    }

    double get_distance_from_point_along_vector(const Point &p, const Vector &v) const
    {
        Line other = Line(p, v);
        pair<bool, Point> intersection_point = get_intersection_point(other);
        if (intersection_point.first)
        {
            return intersection_point.second.distance(p);
        }
        else
        {
            // now the lines are parallel
            return get_distance_from_point(p);
        }
    }

    double get_distance_from_point_along_vector(const Point &p, const Vector &v) const
    {
        Line other = Line(p, v);
        std::pair<bool, Point> intersection_point = get_intersection_point(other);
        if (intersection_point.first)
            return intersection_point.second.distance(p);
        else
            return get_distance_from_point(p);
    }

    double get_distance_from_line(const Line &l) const
    {
        Vector cross = direction.cross(l.direction).normalize();
        Vector b = Vector(p0.x - l.p0.x, p0.y - l.p0.y, p0.z - l.p0.z);
        return fabs(b.dot(cross));
    }

    friend ostream &operator<<(std::ostream &os, const Line &l)
    {
        os << l.p0 << " -> " << l.p1;
        return os;
    }
};