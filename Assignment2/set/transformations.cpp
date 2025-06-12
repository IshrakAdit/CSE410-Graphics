#include "triangle.cpp"

using namespace std;

Matrix translationMatrix(double x, double y, double z)
{
    Matrix result = generateIdentityMatrix(4);

    result.elements[0][3] = x;
    result.elements[1][3] = y;
    result.elements[2][3] = z;

    return result;
}

Matrix scalingMatrix(double x, double y, double z)
{
    Matrix result = generateIdentityMatrix(4);

    result.elements[0][0] = x;
    result.elements[1][1] = y;
    result.elements[2][2] = z;

    return result;
}

Matrix rotationMatrix(double x, double y, double z, double angle)
{
    Matrix result = generateIdentityMatrix(4);

    Vector axis = Vector(x, y, z);
    axis = axis.normalize();

    vector<Vector> bases(3);
    bases[0] = Vector(1, 0, 0);
    bases[1] = Vector(0, 1, 0);
    bases[2] = Vector(0, 0, 1);

    Vector v, v_rotated;
    for (int i = 0; i < 3; i++)
    {
        v = bases[i];
        v_rotated = v.rotate(axis, angle);

        result.elements[0][i] = v_rotated.x;
        result.elements[1][i] = v_rotated.y;
        result.elements[2][i] = v_rotated.z;
    }

    return result;
}

Matrix viewMatrix(Vector eye, Vector look, Vector up)
{
    Vector look_direction_vector = look - eye;
    look_direction_vector = look_direction_vector.normalize();

    Vector right_vector = look_direction_vector.cross(up);
    right_vector = right_vector.normalize();

    Vector up_vector = right_vector.cross(look_direction_vector);
    up_vector = up_vector.normalize();

    Matrix view = translationMatrix(-eye.x, -eye.y, -eye.z);

    Matrix rotation(4, 4);
    rotation.elements[0][0] = right_vector.x, rotation.elements[0][1] = right_vector.y,
    rotation.elements[0][2] = right_vector.z;
    rotation.elements[1][0] = up_vector.x, rotation.elements[1][1] = up_vector.y,
    rotation.elements[1][2] = up_vector.z;
    rotation.elements[2][0] = -look_direction_vector.x, rotation.elements[2][1] = -look_direction_vector.y,
    rotation.elements[2][2] = -look_direction_vector.z;
    rotation.elements[3][3] = 1;

    view = rotation * view;

    return view;
}

Matrix projectionMatrix(double fov_y, double aspect_ratio, double near, double far)
{
    double fov_x = fov_y * aspect_ratio;
    double t = near * tan(fov_y * PI / 360.0);
    double r = near * tan(fov_x * PI / 360.0);

    Matrix projection(4, 4);

    projection.elements[0][0] = near / r;
    projection.elements[1][1] = near / t;
    projection.elements[2][2] = -(far + near) / (far - near);
    projection.elements[2][3] = -(2 * far * near) / (far - near);
    projection.elements[3][2] = -1;

    return projection;
}