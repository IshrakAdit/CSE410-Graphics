#include <iostream>
#include <vector>

#include "vector.cpp"
#include "matrix.cpp"

class Triangle
{
public:
    vector<Matrix> vertices;

    Triangle()
    {
        vertices.assign(3, Matrix(4, 1));
        for (Matrix &v : vertices)
            v.data[3][0] = 1;
    }

    Triangle(const Vector &v1, const Vector &v2, const Vector &v3)
    {
        vertices.assign(3, Matrix(4, 1));

        vertices[0].data[0][0] = v1.x, vertices[0].data[1][0] = v1.y,
        vertices[0].data[2][0] = v1.z, vertices[0].data[3][0] = 1;

        vertices[1].data[0][0] = v2.x, vertices[1].data[1][0] = v2.y,
        vertices[1].data[2][0] = v2.z, vertices[1].data[3][0] = 1;

        vertices[2].data[0][0] = v3.x, vertices[2].data[1][0] = v3.y,
        vertices[2].data[2][0] = v3.z, vertices[2].data[3][0] = 1;
    }

    void transform(const Matrix &m)
    {
        for (Matrix &v : vertices)
        {
            v = m * v;
            v = v / v.data[3][0];
        }
    }

    friend istream &operator>>(istream &is, Triangle &t)
    {
        for (Matrix &v : t.vertices)
        {
            is >> v.data[0][0] >> v.data[1][0] >> v.data[2][0];
            v.data[3][0] = 1;
        }
        return is;
    }

    friend ostream &operator<<(ostream &os, const Triangle &t)
    {
        for (int i = 0; i < 3; i++)
        {
            os << t.vertices[i].data[0][0] << " " << t.vertices[i].data[1][0] << " "
               << t.vertices[i].data[2][0];
            if (i != 2)
                os << endl;
        }
        return os;
    }
};