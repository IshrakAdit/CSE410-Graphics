#ifdef WIN32
#include <windows.h>
#endif

#include <GL/freeglut.h>
#include <GL/glut.h>
#include <bits/stdc++.h>

#include "classes.cpp"
#include "bitmap_image.hpp"

using namespace std;
using namespace chrono;

string input_file;
bool use_multithreading = true;
unsigned int num_threads = thread::hardware_concurrency();

bool texture_mode = false;

int reflection_depth;
int image_width, image_height;
double view_angle = 80;
double far_plane_distance = 500.0;
int captured_images;

Camera camera(Vector(125, -125, 125), Vector(0, 0, 0), Vector(0, 0, 1), 2, 0.5);
vector<Object *> objects;
vector<LightSource *> light_sources;

void initialize();
void display_scene();
void idle_callback();
void key_input(unsigned char key, int x, int y);
void special_key_input(int key, int x, int y);
void load_scene(const string &filename);
void save_rendered_image();
void draw_coordinate_axes();
void cleanup_resources();

void initialize()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    load_scene(input_file);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(view_angle, 1.0, 1.0, far_plane_distance);
}

void draw_coordinate_axes()
{
    glBegin(GL_LINES);
    glLineWidth(20);
    glColor3f(1, 0, 0);
    glVertex3f(200, 0, 0);
    glVertex3f(0, 0, 0);
    glColor3f(0, 1, 0);
    glVertex3f(0, 200, 0);
    glVertex3f(0, 0, 0);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 200);
    glVertex3f(0, 0, 0);
    glEnd();
    glLineWidth(1);
}

void save_rendered_image()
{
    auto start = steady_clock::now();
    bitmap_image image(image_width, image_height);
    image.set_all_channels(0, 0, 0);

    double plane_distance = 1.0;
    double window_size = 2 * tan(view_angle * PI / 360.0) * plane_distance;
    Vector top_left = camera.pos + plane_distance * camera.look -
                      (window_size / 2.0) * camera.right +
                      (window_size / 2.0) * camera.up +
                      0.5 * (camera.right * (window_size / image_width)) -
                      0.5 * (camera.up * (window_size / image_height));

    auto render_range = [&](int start_col, int end_col)
    {
        for (int i = start_col; i < end_col; ++i)
        {
            for (int j = 0; j < image_height; ++j)
            {
                Vector pixel = top_left + i * (window_size / image_width) * camera.right -
                               j * (window_size / image_height) * camera.up;
                Ray ray(pixel, pixel - camera.pos);

                int closest = -1;
                double min_t = 1e9;
                for (int k = 0; k < objects.size(); ++k)
                {
                    double t = objects[k]->find_ray_intersection(ray);
                    if (t > 0 && t < min_t)
                    {
                        min_t = t;
                        closest = k;
                    }
                }

                if (closest == -1)
                    continue;
                double dist = camera.look.dot(min_t * ray.dir);
                if (dist > far_plane_distance)
                    continue;

                Color color(0, 0, 0);
                objects[closest]->shade(ray, color, reflection_depth);
                color.clamp();

                image.set_pixel(i, j, 255 * color.r, 255 * color.g, 255 * color.b);
            }
        }
    };

    if (use_multithreading && num_threads > 1)
    {
        vector<thread> workers;
        int cols_per_thread = image_width / num_threads;
        for (int i = 0; i < num_threads; ++i)
        {
            int start = i * cols_per_thread;
            int end = (i == num_threads - 1) ? image_width : (i + 1) * cols_per_thread;
            workers.emplace_back(render_range, start, end);
        }
        for (auto &t : workers)
            t.join();
    }
    else
    {
        render_range(0, image_width);
    }

    string filename = "Output_" + to_string(++captured_images) + ".bmp";
    image.save_image(filename);

    double elapsed = duration_cast<milliseconds>(
                         steady_clock::now() - start)
                         .count();
    cout << "Saved: " << filename << " in " << elapsed / 1000 << " seconds" << endl;
}

void cleanup_resources()
{
    for (auto *o : objects)
        delete o;
    objects.clear();
    for (auto *l : light_sources)
        delete l;
    light_sources.clear();
}

void load_scene(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open input file." << endl;
        return;
    }

    int resolution;
    file >> reflection_depth >> resolution;
    image_width = image_height = resolution;

    int num_objects;
    file >> num_objects;
    while (num_objects--)
    {
        string type;
        file >> type;

        if (type == "sphere")
        {
            double x, y, z, r, g, b, rad, amb, diff, spec, refl;
            int shine;
            file >> x >> y >> z >> rad >> r >> g >> b >> amb >> diff >> spec >> refl >> shine;
            Object *o = new Sphere(Vector(x, y, z), rad);
            o->set_color(r, g, b);
            o->set_coefficients(amb, diff, spec, refl);
            o->set_shine(shine);
            objects.push_back(o);
        }
        else if (type == "triangle")
        {
            double pts[9], r, g, b, amb, diff, spec, refl;
            int shine;
            for (int i = 0; i < 9; ++i)
                file >> pts[i];
            file >> r >> g >> b >> amb >> diff >> spec >> refl >> shine;
            Object *o = new Triangle(Vector(pts[0], pts[1], pts[2]),
                                     Vector(pts[3], pts[4], pts[5]),
                                     Vector(pts[6], pts[7], pts[8]));
            o->set_color(r, g, b);
            o->set_coefficients(amb, diff, spec, refl);
            o->set_shine(shine);
            objects.push_back(o);
        }
        else if (type == "general")
        {
            double coeff[10], pos[3], dims[3], r, g, b, amb, diff, spec, refl;
            int shine;
            for (int i = 0; i < 10; ++i)
                file >> coeff[i];
            for (int i = 0; i < 3; ++i)
                file >> pos[i];
            for (int i = 0; i < 3; ++i)
                file >> dims[i];
            file >> r >> g >> b >> amb >> diff >> spec >> refl >> shine;
            Object *o = new GeneralQuadraticSurface(coeff[0], coeff[1], coeff[2], coeff[3], coeff[4],
                                                    coeff[5], coeff[6], coeff[7], coeff[8], coeff[9],
                                                    Vector(pos[0], pos[1], pos[2]), dims[0], dims[1], dims[2]);
            o->set_color(r, g, b);
            o->set_coefficients(amb, diff, spec, refl);
            o->set_shine(shine);
            objects.push_back(o);
        }
        else
        {
            cerr << "Unknown object type: " << type << endl;
        }
    }

    Object *floor = new Floor(1000, 20);
    floor->set_coefficients(0.4, 0.2, 0.2, 0.2);
    floor->set_shine(1);
    objects.push_back(floor);

    int num_point_lights;
    file >> num_point_lights;
    while (num_point_lights--)
    {
        double x, y, z, r, g, b;
        file >> x >> y >> z >> r >> g >> b;
        light_sources.push_back(new PointLight(Vector(x, y, z), r, g, b));
    }

    int num_spot_lights;
    file >> num_spot_lights;
    while (num_spot_lights--)
    {
        double x, y, z, r, g, b, dx, dy, dz, angle;
        file >> x >> y >> z >> r >> g >> b >> dx >> dy >> dz >> angle;
        light_sources.push_back(new SpotLight(Vector(x, y, z), r, g, b, Vector(dx, dy, dz), angle));
    }

    file.close();
}

void display_scene()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.pos.x, camera.pos.y, camera.pos.z,
              camera.pos.x + camera.look.x, camera.pos.y + camera.look.y, camera.pos.z + camera.look.z,
              camera.up.x, camera.up.y, camera.up.z);

    for (auto *o : objects)
        o->draw();
    glutSwapBuffers();
}

void idle_callback()
{
    glutPostRedisplay();
}

void key_input(unsigned char key, int, int)
{
    switch (key)
    {
    case '0':
        save_rendered_image();
        break;
    case '1':
        camera.look_left();
        break;
    case '2':
        camera.look_right();
        break;
    case '3':
        camera.look_up();
        break;
    case '4':
        camera.look_down();
        break;
    case '5':
        camera.tilt_clockwise();
        break;
    case '6':
        camera.tilt_counterclockwise();
        break;
    case 'w':
        camera.move_up_same_ref();
        break;
    case 's':
        camera.move_down_same_ref();
        break;
    case 'p':
        cout << "Camera Position: ("
             << fixed << setprecision(2)
             << camera.pos.x << ", "
             << camera.pos.y << ", "
             << camera.pos.z << ")\n";
        break;
    case 't':
        texture_mode = !texture_mode;
        cout << "Texture mode: " << (texture_mode ? "ON" : "OFF") << endl;
        break;
    default:
        cout << "Unknown key pressed" << endl;
        break;
    }
}

void special_key_input(int key, int, int)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        camera.move_forward();
        break;
    case GLUT_KEY_DOWN:
        camera.move_backward();
        break;
    case GLUT_KEY_LEFT:
        camera.move_left();
        break;
    case GLUT_KEY_RIGHT:
        camera.move_right();
        break;
    case GLUT_KEY_PAGE_UP:
        camera.move_up();
        break;
    case GLUT_KEY_PAGE_DOWN:
        camera.move_down();
        break;
    default:
        cout << "Unknown key pressed" << endl;
        break;
    }
}

int main(int argc, char **argv)
{
    input_file = (argc < 2) ? "scene.txt" : argv[1];
    loadTexture("texture1.jpg");

    glutInit(&argc, argv);
    glutInitWindowSize(768, 768);
    glutInitWindowPosition(100, 100);
    glEnable(GLUT_MULTISAMPLE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow("Ray Tracing");

    glutDisplayFunc(display_scene);
    glutKeyboardFunc(key_input);
    glutSpecialFunc(special_key_input);
    glutIdleFunc(idle_callback);

    initialize();

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glutCloseFunc(cleanup_resources);
    glutMainLoop();

    if (textureData)
    {
        stbi_image_free(textureData);
        textureData = nullptr;
    }

    return 0;
}
