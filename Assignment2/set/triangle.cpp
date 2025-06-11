#include "data_structures.cpp"

static unsigned long long int g_seed = 1;
inline int fastrand()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

class Triangle
{
public:
    vector<Matrix> vertices;
    unsigned char red, green, blue;

    Triangle()
    {
        vertices.assign(3, Matrix(4, 1));
        for (Matrix &v : vertices)
            v.elements[3][0] = 1;
    }

    void transform(const Matrix &m)
    {
        for (Matrix &v : vertices)
        {
            v = m * v;
            v = v / v.elements[3][0];
        }
    }

    void set_random_colors()
    {
        red = fastrand() % 256;
        green = fastrand() % 256;
        blue = fastrand() % 256;
    }

    void reorder_vertices()
    {
        sort(vertices.begin(), vertices.end(),
             [](const Matrix &a, const Matrix &b)
             {
                 return a.elements[1][0] > b.elements[1][0];
             });
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