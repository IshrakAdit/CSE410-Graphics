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
            v.elements[3][0] = 1;
    }

    Triangle(const Vector &v1, const Vector &v2, const Vector &v3)
    {
        vertices.assign(3, Matrix(4, 1));

        vertices[0].elements[0][0] = v1.x, vertices[0].elements[1][0] = v1.y,
        vertices[0].elements[2][0] = v1.z, vertices[0].elements[3][0] = 1;

        vertices[1].elements[0][0] = v2.x, vertices[1].elements[1][0] = v2.y,
        vertices[1].elements[2][0] = v2.z, vertices[1].elements[3][0] = 1;

        vertices[2].elements[0][0] = v3.x, vertices[2].elements[1][0] = v3.y,
        vertices[2].elements[2][0] = v3.z, vertices[2].elements[3][0] = 1;
    }

    void transform(const Matrix &m)
    {
        for (Matrix &v : vertices)
        {
            v = m * v;
            v = v / v.elements[3][0];
        }
    }

    friend istream &operator>>(istream &is, Triangle &t)
    {
        for (Matrix &v : t.vertices)
        {
            is >> v.elements[0][0] >> v.elements[1][0] >> v.elements[2][0];
            v.elements[3][0] = 1;
        }
        return is;
    }

    friend ostream &operator<<(ostream &os, const Triangle &t)
    {
        for (int i = 0; i < 3; i++)
        {
            os << t.vertices[i].elements[0][0] << " " << t.vertices[i].elements[1][0] << " "
               << t.vertices[i].elements[2][0];
            if (i != 2)
                os << endl;
        }
        return os;
    }
};