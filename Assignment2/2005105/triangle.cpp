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
            v = m * v;                // Transformation
            v = v / v.elements[3][0]; // Normalization
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
                 // Ordered by y-cordinates
                 return a.elements[1][0] > b.elements[1][0];
             });
    }

    friend istream &operator>>(istream &input_stream, Triangle &triangle)
    {
        for (Matrix &v : triangle.vertices)
        {
            input_stream >> v.elements[0][0] >> v.elements[1][0] >> v.elements[2][0];
            v.elements[3][0] = 1;
        }
        return input_stream;
    }

    friend ostream &operator<<(ostream &output_stream, const Triangle &triangle)
    {
        for (int i = 0; i < 3; i++)
        {
            output_stream << triangle.vertices[i].elements[0][0] << " " << triangle.vertices[i].elements[1][0] << " "
                          << triangle.vertices[i].elements[2][0];
            if (i != 2)
                output_stream << endl;
        }
        return output_stream;
    }
};