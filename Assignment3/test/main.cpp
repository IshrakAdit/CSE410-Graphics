#ifdef WIN32
#include <windows.h>
#endif

#include <GL/freeglut.h>
#include <GL/glut.h>
#include <bits/stdc++.h>

#include "classes.cpp"
#include "bitmap_image.hpp"

std::string input_file;
bool use_multithreading = true;
unsigned int num_threads = std::thread::hardware_concurrency();

int reflection_depth;
int image_width, image_height;
double view_angle = 80; // in degrees
double far_plane_distance = 500.0;
int captured_images;

Camera camera(Vector(125, -125, 125), Vector(0, 0, 0), Vector(0, 0, 1), 2, 0.5);
std::vector<Object *> objects;
std::vector<LightSource *> light_sources;

// Function Declarations
void init();
void display();
void idle();
void handle_keys(unsigned char key, int x, int y);
void handle_special_keys(int key, int x, int y);
void load_data(const std::string &filename);
void capture();
void draw_axes();
void free_memory();

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f,
                 1.0f); // Set background color to black and opaque

    load_data(input_file);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(view_angle, 1.0, 1.0, far_plane_distance);
}

void draw_axes()
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

void capture()
{
    std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();
    bitmap_image image(image_width, image_height);
    image.set_all_channels(0, 0, 0);

    // plane_distance is the distance from the camera to the image plane
    double plane_distance = 1.0;
    double window_height = 2 * tan(view_angle * PI / 360.0) * plane_distance;
    double window_width = window_height;
    Vector top_left = camera.pos + plane_distance * camera.look -
                      (window_width / 2.0) * camera.right +
                      (window_height / 2.0) * camera.up;
    double du = window_width / image_width;
    double dv = window_height / image_height;

    // Choose middle of the grid cell
    top_left += 0.5 * du * camera.right - 0.5 * dv * camera.up;

    auto render_segment = [&](int start_col, int end_col)
    {
        for (int i = start_col; i < end_col; i++)
        {
            for (int j = 0; j < image_height; j++)
            {
                // Calculate current pixel
                Vector cur_pixel =
                    top_left + i * du * camera.right - j * dv * camera.up;

                // Cast ray from eye to pixel
                Ray ray(cur_pixel, cur_pixel - camera.pos);

                int nearest_idx = -1;
                double t_min = 1e9;
                for (int k = 0; k < objects.size(); k++)
                {
                    Object *o = objects[k];
                    double t = o->find_ray_intersection(ray);
                    if (t > 0 && t < t_min)
                    {
                        t_min = t;
                        nearest_idx = k;
                    }
                }

                if (nearest_idx == -1)
                    continue;
                double dist = camera.look.dot(t_min * ray.dir);
                if (dist > far_plane_distance)
                    continue;
                Color color(0, 0, 0);
                objects[nearest_idx]->shade(ray, color, reflection_depth);
                color.clamp();

                image.set_pixel(i, j, 255 * color.r, 255 * color.g,
                                255 * color.b);
            }
        }
    };

    if (use_multithreading && num_threads > 1)
    {
        std::vector<std::thread> threads;
        int cols_per_thread = image_width / num_threads;
        for (int i = 0; i < num_threads; i++)
        {
            int start_col = i * cols_per_thread;
            int end_col = (i == num_threads - 1) ? image_width
                                                 : (i + 1) * cols_per_thread;
            threads.push_back(std::thread(render_segment, start_col, end_col));
        }
        for (auto &t : threads)
            t.join();
    }
    else
    {
        render_segment(0, image_width);
    }

    std::string output_file =
        "Output_1" + std::to_string(++captured_images) + ".bmp";

    image.save_image(output_file);
    double time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now() - start)
                              .count();
    std::cout << "Image captured to " << output_file << " in "
              << time_elapsed / 1000 << " seconds" << std::endl;
}

void free_memory()
{
    for (Object *object : objects)
        delete object;
    objects.clear();

    for (LightSource *light : light_sources)
        delete light;
    light_sources.clear();
}

void load_data(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: File not found" << std::endl;
        return;
    }

    int pixel;
    file >> reflection_depth >> pixel;
    image_width = image_height = pixel;

    int num_objects;
    file >> num_objects;

    for (int i = 0; i < num_objects; i++)
    {
        std::string type;
        file >> type;

        if (type == "sphere")
        {
            double x, y, z, radius;
            file >> x >> y >> z >> radius;
            double r, g, b;
            file >> r >> g >> b;
            double ambient, diffuse, specular, reflection;
            file >> ambient >> diffuse >> specular >> reflection;
            int shine;
            file >> shine;
            Vector center(x, y, z);
            Object *temp = new Sphere(center, radius);
            temp->set_color(r, g, b);
            temp->set_coefficients(ambient, diffuse, specular, reflection);
            temp->set_shine(shine);
            objects.push_back(temp);
        }
        else if (type == "triangle")
        {
            double x1, y1, z1, x2, y2, z2, x3, y3, z3;
            file >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> x3 >> y3 >> z3;
            Vector p1(x1, y1, z1), p2(x2, y2, z2), p3(x3, y3, z3);
            double r, g, b;
            file >> r >> g >> b;
            double ambient, diffuse, specular, reflection;
            file >> ambient >> diffuse >> specular >> reflection;
            int shine;
            file >> shine;
            Object *temp = new Triangle(p1, p2, p3);
            temp->set_color(r, g, b);
            temp->set_coefficients(ambient, diffuse, specular, reflection);
            temp->set_shine(shine);
            objects.push_back(temp);
        }
        else if (type == "general")
        {
            double A, B, C, D, E, F, G, H, I, J;
            file >> A >> B >> C >> D >> E >> F >> G >> H >> I >> J;
            double x, y, z;
            file >> x >> y >> z;
            double length, width, height;
            file >> length >> width >> height;
            double r, g, b;
            file >> r >> g >> b;
            double ambient, diffuse, specular, reflection;
            file >> ambient >> diffuse >> specular >> reflection;
            int shine;
            file >> shine;
            Vector reference_point(x, y, z);
            Object *temp = new GeneralQuadraticSurface(A, B, C, D, E, F, G, H,
                                                       I, J, reference_point,
                                                       length, width, height);
            temp->set_color(r, g, b);
            temp->set_coefficients(ambient, diffuse, specular, reflection);
            temp->set_shine(shine);
            objects.push_back(temp);
        }
        else
        {
            std::cerr << "Error reading file: Unknown object type" << std::endl;
            return;
        }
    }

    // The Floor
    Object *floor = new Floor(1000, 20);
    floor->set_coefficients(0.4, 0.2, 0.2, 0.2);
    floor->set_shine(1);
    objects.push_back(floor);

    // Point Light Sources
    int num_point_lights;
    file >> num_point_lights;
    for (int i = 0; i < num_point_lights; i++)
    {
        double x, y, z;
        file >> x >> y >> z;
        double r, g, b;
        file >> r >> g >> b;
        Vector position(x, y, z);
        LightSource *pl = new PointLight(position, r, g, b);
        light_sources.push_back(pl);
    }

    // Spot Light Sources
    int num_spot_lights;
    file >> num_spot_lights;
    for (int i = 0; i < num_spot_lights; i++)
    {
        double x, y, z;
        file >> x >> y >> z;
        double r, g, b;
        file >> r >> g >> b;
        Vector position(x, y, z);
        double direction_x, direction_y, direction_z;
        file >> direction_x >> direction_y >> direction_z;
        double angle;
        file >> angle;
        Vector direction(direction_x, direction_y, direction_z);
        LightSource *sl = new SpotLight(position, r, g, b, direction, angle);
        light_sources.push_back(sl);
    }

    file.close();
}

void display()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.pos.x, camera.pos.y, camera.pos.z,
              camera.pos.x + camera.look.x, camera.pos.y + camera.look.y,
              camera.pos.z + camera.look.z, camera.up.x, camera.up.y,
              camera.up.z);
    // draw_axes();
    for (Object *they : objects)
        they->draw();
    // for (LightSource *light : light_sources) light->draw();
    glutSwapBuffers();
}

void idle() { glutPostRedisplay(); }

void handle_keys(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '0':
        capture();
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
        printf("Camera Position: (%.2lf, %.2lf, %.2lf)\n", camera.pos.x,
               camera.pos.y, camera.pos.z);
        break;
    default:
        printf("Unknown key pressed\n");
        break;
    }
}

void handle_special_keys(int key, int x, int y)
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
        printf("Unknown key pressed\n");
        break;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
        input_file = "scene.txt";
    else
        input_file = argv[1];

    glutInit(&argc, argv);
    glutInitWindowSize(768, 768);
    glutInitWindowPosition(100, 100);
    glEnable(GLUT_MULTISAMPLE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow("Ray Tracing");
    glutDisplayFunc(display);
    glutKeyboardFunc(handle_keys);
    glutSpecialFunc(handle_special_keys);
    glutIdleFunc(idle);
    init();

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glutCloseFunc(free_memory);

    glutMainLoop();

    return 0;
}