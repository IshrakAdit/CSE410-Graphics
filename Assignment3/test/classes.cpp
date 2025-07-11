#include <GL/glut.h>
#include <bits/stdc++.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

const double PI = 2 * acos(0.0);
const double EPS = 1e-6;

class Object;
struct LightSource;

extern vector<Object *> objects;
extern vector<LightSource *> light_sources;

extern bool texture_mode;
unsigned char *textureData = nullptr;
int textureWidth = 0, textureHeight = 0, textureChannels = 0;

struct Color
{
    double r, g, b;
    Color(double rr = 0, double gg = 0, double bb = 0)
    {
        r = rr, g = gg, b = bb;
    }

    Color operator+(const Color &other) const
    {
        return Color(r + other.r, g + other.g, b + other.b);
    }

    Color operator+=(const Color &other)
    {
        r += other.r;
        g += other.g;
        b += other.b;
        return *this;
    }

    Color operator*(const double &val) const
    {
        return Color(r * val, g * val, b * val);
    }

    Color operator*(const Color &other) const
    {
        return Color(r * other.r, g * other.g, b * other.b);
    }

    Color operator*=(const double &val)
    {
        r *= val;
        g *= val;
        b *= val;
        return *this;
    }

    void clamp()
    {
        r = max(0.0, min(1.0, r));
        g = max(0.0, min(1.0, g));
        b = max(0.0, min(1.0, b));
    }

    friend Color operator*(const double &val, const Color &col)
    {
        return Color(col.r * val, col.g * val, col.b * val);
    }
};

void loadTexture(const string &filename)
{
    textureData = stbi_load(filename.c_str(), &textureWidth, &textureHeight, &textureChannels, 3);
    if (!textureData)
    {
        cerr << "Failed to load texture image: " << filename << endl;
        textureWidth = textureHeight = 0;
    }
    else
    {
        cout << "Loaded texture: " << filename << " (" << textureWidth << "x" << textureHeight << ")" << endl;
    }
}

Color sampleTexture(double u, double v)
{
    if (!textureData || textureWidth <= 0 || textureHeight <= 0)
        return Color(0.5, 0.5, 0.5); // Fallback gray

    u = std::max(0.0, std::min(1.0, u));
    v = std::max(0.0, std::min(1.0, v));

    int pixel_x = (int)(u * (textureWidth - 1));
    int pixel_y = (int)((1.0 - v) * (textureHeight - 1));

    int index = (pixel_y * textureWidth + pixel_x) * 3;

    double r = textureData[index] / 255.0;
    double g = textureData[index + 1] / 255.0;
    double b = textureData[index + 2] / 255.0;

    return Color(r, g, b);
}

struct PhongCoefficients
{
    double ambient, diffuse, specular, reflection;
    int shine;

    PhongCoefficients(double a = 0, double d = 0, double s = 0, double refl = 0, int sh = 0)
        : ambient(a), diffuse(d), specular(s), reflection(refl), shine(sh) {}
};

struct Vector
{
    double x, y, z;

    Vector(double a = 0, double b = 0, double c = 0) : x(a), y(b), z(c) {}

    Vector operator+(const Vector &v) const
    {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector &operator+=(const Vector &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector operator-(const Vector &v) const
    {
        return Vector(x - v.x, y - v.y, z - v.z);
    }

    Vector operator-() const
    {
        return Vector(-x, -y, -z);
    }

    Vector &operator-=(const Vector &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector operator*(const double &val) const
    {
        return Vector(x * val, y * val, z * val);
    }

    friend Vector operator*(const double &val, const Vector &v)
    {
        return Vector(v.x * val, v.y * val, v.z * val);
    }

    Vector operator/(const double &val) const
    {
        if (abs(val) <= EPS)
            throw invalid_argument("Division by zero attempted in Vector.");
        return Vector(x / val, y / val, z / val);
    }

    const Vector &operator=(const Vector &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    double dot(const Vector &v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    Vector cross(const Vector &v) const
    {
        return Vector(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x);
    }

    Vector normalize() const
    {
        double magnitude = sqrt(x * x + y * y + z * z);
        if (magnitude <= EPS)
            throw invalid_argument("Normalization of zero magnitude vector.");
        return *this / magnitude;
    }

    Vector rotate(const Vector &axis, double angle_deg) const
    {
        double theta = angle_deg * PI / 180.0;
        Vector k = axis.normalize();
        Vector v1 = (*this) * cos(theta);
        Vector v2 = k.cross(*this) * sin(theta);
        Vector v3 = k * (k.dot(*this)) * (1 - cos(theta));
        return v1 + v2 + v3;
    }

    double norm() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    friend double distance(const Vector &a, const Vector &b)
    {
        return (a - b).norm();
    }

    friend istream &operator>>(istream &in, Vector &v)
    {
        return in >> v.x >> v.y >> v.z;
    }

    friend ostream &operator<<(ostream &out, const Vector &v)
    {
        return out << "Vector(" << v.x << ", " << v.y << ", " << v.z << ")";
    }
};

struct Camera
{
    double speed, rotation_speed;
    Vector pos;
    Vector up, right, look;

    Camera(Vector eye = Vector(5, 5, 5), Vector center = Vector(0, 0, 0), Vector up_vec = Vector(0, 0, 1), double sp = 2.0, double rs = 0.5)
        : speed(sp), rotation_speed(rs), pos(eye)
    {
        look = (center - eye).normalize();
        if (abs(look.dot(up_vec)) <= EPS)
        {
            up = up_vec.normalize();
            right = look.cross(up).normalize();
        }
        else
        {
            right = Vector(look.y, -look.x, 0).normalize();
            up = right.cross(look).normalize();
        }
    }

    void move_forward() { pos += look * speed; }
    void move_backward() { pos -= look * speed; }
    void move_left() { pos -= right * speed; }
    void move_right() { pos += right * speed; }
    void move_up() { pos += up * speed; }
    void move_down() { pos -= up * speed; }

    void look_left()
    {
        look = look.rotate(up, rotation_speed);
        right = right.rotate(up, rotation_speed);
    }

    void look_right()
    {
        look = look.rotate(up, -rotation_speed);
        right = right.rotate(up, -rotation_speed);
    }

    void look_up()
    {
        look = look.rotate(right, rotation_speed);
        up = up.rotate(right, rotation_speed);
    }

    void look_down()
    {
        look = look.rotate(right, -rotation_speed);
        up = up.rotate(right, -rotation_speed);
    }

    void tilt_clockwise()
    {
        right = right.rotate(look, rotation_speed);
        up = up.rotate(look, rotation_speed);
    }

    void tilt_counterclockwise()
    {
        right = right.rotate(look, -rotation_speed);
        up = up.rotate(look, -rotation_speed);
    }

    void move_up_same_ref()
    {
        double prev_len = distance(pos, Vector(0, 0, 0));
        pos.z += speed;
        double new_len = distance(pos, Vector(0, 0, 0));
        double angle = acos((prev_len * prev_len + new_len * new_len - speed * speed) / (2 * prev_len * new_len));
        angle = angle * 180.0 / PI;
        look = look.rotate(right, -angle);
        up = up.rotate(right, -angle);
        right = look.cross(up).normalize();
    }

    void move_down_same_ref()
    {
        double prev_len = distance(pos, Vector(0, 0, 0));
        pos.z -= speed;
        double new_len = distance(pos, Vector(0, 0, 0));
        double angle = acos((prev_len * prev_len + new_len * new_len - speed * speed) / (2 * prev_len * new_len));
        angle = angle * 180.0 / PI;
        look = look.rotate(right, angle);
        up = up.rotate(right, angle);
        right = look.cross(up).normalize();
    }
};

struct Ray
{
    Vector origin, dir;

    Ray(const Vector &start, const Vector &direction)
    {
        origin = start;
        dir = direction.normalize();
    }
};

struct LightSource
{
    Color color;
    Vector light_position;

    enum LightType
    {
        POINT,
        SPOT
    } type;

    LightSource(const Vector &pos, double r, double g, double b, LightType t)
        : light_position(pos), color(r, g, b), type(t) {}

    virtual void draw() const = 0;
    virtual ~LightSource() {}
};

struct PointLight : public LightSource
{
    PointLight(const Vector &pos, double r, double g, double b)
        : LightSource(pos, r, g, b, POINT) {}

    void draw() const override
    {
        glColor3f(1, 1, 0);
        glPushMatrix();
        glTranslatef(light_position.x, light_position.y, light_position.z);
        glutSolidSphere(4, 50, 50);
        glPopMatrix();
    }
};

struct SpotLight : public LightSource
{
    Vector light_direction;
    double cutoff_angle;

    SpotLight(const Vector &pos, double r, double g, double b,
              const Vector &dir, double angle)
        : LightSource(pos, r, g, b, SPOT), cutoff_angle(angle)
    {
        light_direction = dir.normalize();
    }

    void draw() const override
    {
        glColor3f(0, 1, 1);
        glPushMatrix();
        glTranslatef(light_position.x, light_position.y, light_position.z);
        glutSolidSphere(2, 50, 50);
        glPopMatrix();
    }
};

class Object
{
protected:
    Vector reference_point;
    Color color;
    PhongCoefficients phong_coefficients;
    // double red_refractive_index, green_refractive_index, blue_refractive_index;

    Vector get_reflection(const Vector &normal, const Vector &incident) const
    {
        return incident - normal * (2.0 * incident.dot(normal));
    }

    Vector get_refraction(const Vector &normal, const Vector &incident, double n1, double n2) const
    {
        double ratio = n1 / n2;
        double cos_i = -normal.dot(incident);
        double cos_t = sqrt(1 - ratio * ratio * (1 - cos_i * cos_i));
        return incident * ratio + normal * (ratio * cos_i - cos_t);
    }

    int get_next_reflection_object(Ray reflected_ray) const
    {
        int nearest_id = -1;
        double min_t = 1e9;
        for (int i = 0; i < objects.size(); i++)
        {
            Object *obj = objects[i];
            double t = obj->find_ray_intersection(reflected_ray);
            if (t > 0 && t < min_t)
            {
                min_t = t;
                nearest_id = i;
            }
        }
        return nearest_id;
    }

public:
    Object(const Vector &ref = Vector(0, 0, 0))
    {
        reference_point = ref;
    }

    virtual Color get_color_at(const Vector &pt) const
    {
        return color;
    }

    virtual void shade(const Ray &ray, Color &col, int level) const
    {
        if (level == 0)
            return;

        double t_hit = find_ray_intersection(ray);
        if (t_hit < 0)
            return;

        Vector intersect = ray.origin + ray.dir * t_hit;
        Color base_color = get_color_at(intersect);
        col = base_color * phong_coefficients.ambient;

        Vector normal = get_normal(intersect);
        if (ray.dir.dot(normal) > 0)
            normal = -normal;

        for (auto light : light_sources)
        {
            Ray l_ray(light->light_position, intersect - light->light_position);

            double beta = 0;
            if (light->type == LightSource::SPOT)
            {
                auto *s = static_cast<SpotLight *>(light);
                double dot_val = l_ray.dir.dot(s->light_direction);
                double theta = acos(dot_val / (l_ray.dir.norm() * s->light_direction.norm())) * 180.0 / PI;
                beta = fabs(theta * PI / 180.0);
                if (fabs(theta) >= s->cutoff_angle)
                    continue;
            }

            double light_distance = (intersect - light->light_position).norm();
            if (light_distance <= EPS)
                continue;

            bool shadowed = false;
            for (auto obj : objects)
            {
                double t_shadow = obj->find_ray_intersection(l_ray);
                if (t_shadow > EPS && t_shadow + EPS < light_distance)
                {
                    shadowed = true;
                    break;
                }
            }
            if (shadowed)
                continue;

            double lambert = max(0.0, normal.dot(-l_ray.dir));
            if (lambert < EPS)
                continue;

            double falloff = (light->type == LightSource::SPOT) ? pow(cos(beta), 2.0) : 1.0;

            col += light->color * phong_coefficients.diffuse * lambert * base_color * falloff;

            Ray r_ray(intersect, get_reflection(normal, l_ray.dir));
            double phong = max(0.0, r_ray.dir.dot(-ray.dir));
            col += light->color * phong_coefficients.specular * pow(phong, phong_coefficients.shine) * base_color * falloff;
        }

        if (level == 0)
            return;

        Ray refl_ray(intersect, get_reflection(normal, ray.dir));
        refl_ray.origin += refl_ray.dir * EPS;

        int next_obj = get_next_reflection_object(refl_ray);
        if (next_obj == -1)
            return;

        Color refl_color(0, 0, 0);
        objects[next_obj]->shade(refl_ray, refl_color, level - 1);
        col += refl_color * phong_coefficients.reflection;
    }

    virtual double find_ray_intersection(Ray ray) const = 0;

    void set_color(double r, double g, double b)
    {
        color = Color(r, g, b);
    }

    void set_shine(int s)
    {
        phong_coefficients.shine = s;
    }

    void set_coefficients(double amb, double diff, double spec, double refl)
    {
        phong_coefficients = PhongCoefficients(amb, diff, spec, refl, phong_coefficients.shine);
    }

    // void set_refractive_indices(double r, double g, double b)
    // {
    //     red_refractive_index = r;
    //     green_refractive_index = g;
    //     blue_refractive_index = b;
    // }

    virtual void draw() const = 0;
    virtual Vector get_normal(const Vector &point) const = 0;
    // virtual void print() const = 0;
    virtual ~Object() {}
};

class Floor : public Object
{
public:
    double floor_width, tile_width;

    Floor(double fw, double tw)
        : Object(Vector(-fw / 2.0, -fw / 2.0, 0.0)), floor_width(fw), tile_width(tw) {}

    void draw() const override
    {
        double x = -floor_width / 2.0;
        double y_start = -floor_width / 2.0;
        int count = floor_width / tile_width;

        glPushMatrix();
        for (int i = 0; i < count; i++)
        {
            double y = y_start;
            for (int j = 0; j < count; j++)
            {
                glColor3f((i + j) % 2 == 0 ? 0 : 1, (i + j) % 2 == 0 ? 0 : 1, (i + j) % 2 == 0 ? 0 : 1);
                glBegin(GL_QUADS);
                glVertex3f(x, y, 0.0);
                glVertex3f(x + tile_width, y, 0.0);
                glVertex3f(x + tile_width, y + tile_width, 0.0);
                glVertex3f(x, y + tile_width, 0.0);
                glEnd();
                y += tile_width;
            }
            x += tile_width;
        }
        glPopMatrix();
    }

    Vector get_normal(const Vector &) const override
    {
        return Vector(0, 0, 1);
    }

    Color get_color_at(const Vector &pt) const override
    {
        if (texture_mode)
        {
            const int repeat_factor = 10;

            double u = (pt.x - reference_point.x) / floor_width;
            double v = (pt.y - reference_point.y) / floor_width;

            u = fmod(u * repeat_factor, 1.0);
            v = fmod(v * repeat_factor, 1.0);

            if (u < 0)
                u += 1.0;
            if (v < 0)
                v += 1.0;

            return sampleTexture(u, v);
        }
        else
        {
            int i = (pt.x - reference_point.x) / tile_width;
            int j = (pt.y - reference_point.y) / tile_width;
            return ((i + j) % 2 == 0) ? Color(0, 0, 0) : Color(1, 1, 1);
        }
    }

    double find_ray_intersection(Ray ray) const override
    {
        Vector n = get_normal(reference_point);
        double denom = n.dot(ray.dir);
        if (fabs(denom) < EPS)
            return -1.0;

        double t = -(n.dot(ray.origin - reference_point)) / denom;
        Vector hit = ray.origin + ray.dir * t;

        if (hit.x < reference_point.x || hit.x > reference_point.x + floor_width ||
            hit.y < reference_point.y || hit.y > reference_point.y + floor_width)
            return -1.0;
        return t;
    }

    // void print() const override
    // {
    //     cout << "Floor at (" << reference_point.x << ", " << reference_point.y << ", " << reference_point.z
    //          << "), floor width: " << floor_width << ", tile width: " << tile_width << endl;
    // }
};

class Sphere : public Object
{
public:
    double radius;

    Sphere(const Vector &center, double r) : Object(center), radius(r) {}

    void draw() const override
    {
        glColor3f(color.r, color.g, color.b);
        glPushMatrix();
        glTranslatef(reference_point.x, reference_point.y, reference_point.z);
        glutSolidSphere(radius, 50, 50);
        glPopMatrix();
    }

    Vector get_normal(const Vector &p) const override
    {
        return (p - reference_point).normalize();
    }

    double find_ray_intersection(Ray ray) const override
    {
        Vector L = ray.origin - reference_point;
        double b = 2 * ray.dir.dot(L);
        double c = L.dot(L) - radius * radius;
        double delta = b * b - 4 * c;

        if (delta < 0)
            return -1.0;

        double t1 = (-b - sqrt(delta)) / 2;
        double t2 = (-b + sqrt(delta)) / 2;

        if (t1 >= 0 && t2 >= 0)
            return min(t1, t2);
        if (t1 >= 0)
            return t1;
        if (t2 >= 0)
            return t2;
        return -1.0;
    }

    // void print() const override
    // {
    //     cout << "Sphere at (" << reference_point.x << ", " << reference_point.y << ", " << reference_point.z
    //          << "), radius: " << radius << endl;
    // }
};

class Triangle : public Object
{
public:
    Vector a, b, c;

    Triangle(const Vector &v1, const Vector &v2, const Vector &v3) : a(v1), b(v2), c(v3) {}

    void draw() const override
    {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_TRIANGLES);
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
        glEnd();
    }

    Vector get_normal(const Vector &) const override
    {
        return (b - a).cross(c - a).normalize();
    }

    double find_ray_intersection(Ray ray) const override
    {
        auto det = [](double m[3][3])
        {
            return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        };

        double beta_mat[3][3] = {
            {a.x - ray.origin.x, a.x - c.x, ray.dir.x},
            {a.y - ray.origin.y, a.y - c.y, ray.dir.y},
            {a.z - ray.origin.z, a.z - c.z, ray.dir.z}};

        double gamma_mat[3][3] = {
            {a.x - b.x, a.x - ray.origin.x, ray.dir.x},
            {a.y - b.y, a.y - ray.origin.y, ray.dir.y},
            {a.z - b.z, a.z - ray.origin.z, ray.dir.z}};

        double t_mat[3][3] = {
            {a.x - b.x, a.x - c.x, a.x - ray.origin.x},
            {a.y - b.y, a.y - c.y, a.y - ray.origin.y},
            {a.z - b.z, a.z - c.z, a.z - ray.origin.z}};

        double A_mat[3][3] = {
            {a.x - b.x, a.x - c.x, ray.dir.x},
            {a.y - b.y, a.y - c.y, ray.dir.y},
            {a.z - b.z, a.z - c.z, ray.dir.z}};

        double A = det(A_mat);
        double beta = det(beta_mat) / A;
        double gamma = det(gamma_mat) / A;
        double t = det(t_mat) / A;

        if (beta > 0 && gamma > 0 && beta + gamma < 1 && t > 0)
            return t;
        return -1.0;
    }

    // void print() const override
    // {
    //     cout << "Triangle vertices: " << a << ", " << b << ", " << c << endl;
    // }
};

class GeneralQuadraticSurface : public Object
{
public:
    double A, B, C, D, E, F, G, H, I, J;
    double length, width, height;

    GeneralQuadraticSurface(double A, double B, double C, double D, double E,
                            double F, double G, double H, double I, double J,
                            const Vector &ref, double l, double w, double h)
        : Object(ref), A(A), B(B), C(C), D(D), E(E), F(F), G(G), H(H), I(I), J(J),
          length(l), width(w), height(h) {}

    void draw() const override {}

    Vector get_normal(const Vector &p) const override
    {
        return Vector(
                   2 * A * p.x + D * p.y + F * p.z + G,
                   2 * B * p.y + D * p.x + E * p.z + H,
                   2 * C * p.z + E * p.y + F * p.x + I)
            .normalize();
    }

    double find_ray_intersection(Ray ray) const override
    {
        double dx = ray.dir.x, dy = ray.dir.y, dz = ray.dir.z;
        double x0 = ray.origin.x, y0 = ray.origin.y, z0 = ray.origin.z;

        double a = A * dx * dx + B * dy * dy + C * dz * dz +
                   D * dx * dy + E * dy * dz + F * dz * dx;

        double b = 2 * (A * x0 * dx + B * y0 * dy + C * z0 * dz) +
                   D * (x0 * dy + y0 * dx) +
                   E * (y0 * dz + z0 * dy) +
                   F * (z0 * dx + x0 * dz) +
                   G * dx + H * dy + I * dz;

        double c = A * x0 * x0 + B * y0 * y0 + C * z0 * z0 +
                   D * x0 * y0 + E * y0 * z0 + F * z0 * x0 +
                   G * x0 + H * y0 + I * z0 + J;

        double disc = b * b - 4 * a * c;
        if (disc < 0)
            return -1.0;

        double t1 = (-b - sqrt(disc)) / (2 * a);
        double t2 = (-b + sqrt(disc)) / (2 * a);

        auto inside_bounds = [&](double t)
        {
            Vector p = ray.origin + ray.dir * t;
            return (!length || (p.x >= reference_point.x - EPS && p.x <= reference_point.x + length + EPS)) &&
                   (!width || (p.y >= reference_point.y - EPS && p.y <= reference_point.y + width + EPS)) &&
                   (!height || (p.z >= reference_point.z - EPS && p.z <= reference_point.z + height + EPS));
        };

        if (t1 > 0 && inside_bounds(t1))
            return t1;
        if (t2 > 0 && inside_bounds(t2))
            return t2;
        return -1.0;
    }

    // void print() const override
    // {
    //     cout << "General Quadratic Surface at (" << reference_point.x << ", "
    //          << reference_point.y << ", " << reference_point.z << ")"
    //          << ", dimensions: " << length << " × " << width << " × " << height << endl;
    // }
};
