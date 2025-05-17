#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define PI (2 * acos(0.0))
#define CUBE_SIZE 100
#define BALL_RADIUS 4.0
#define NUMBER_OF_BALL_STRIPES 30
#define GRAVITY 9.8
#define RESTITUTION 0.75
#define MIN_VELOCITY 2
#define ARROW_SCALE 2

float rotang = 0, bladespeed = 2, bladeang = 0;

// Support structures
struct point
{
    double x, y, z;
};

// Variables: Camera & directions
point camera_position = {70, 70, 80};
point look_direction_vector = {-1, -1, -1};
point right_vector = {1, -1, 0};
point up_vector = {0, 0, 1};
point z_vector = {0, 0, 1};

// Variables: Environment
double animation_angle = 0;

// Variables: Cube
point cube_center = {0, 0, 0};

// Variables: Ball position and velocity
point ball_position = {0, 0, BALL_RADIUS};

// Variables: Ball velocity
point ball_velocity = {0, 0, 0};
point initial_velocity = {0, 0, 30};
double initial_speed = 30.0;

// Variables: Ball movement
double z_axis_shift = 8;

// Variables: Ball rotation
double ball_rotation_angle = 0;
point ball_rotation_axis = {1, 0, 0};

// Variables: simulation
int draw_grid = 0;
int draw_axes = 0;
int simulation_running = 0;
int show_velocity_arrow = 1;

// Utility functions
void normalize(point *p)
{
    double len = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
    p->x /= len;
    p->y /= len;
    p->z /= len;
}

double magnitude(point p)
{
    return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

double magnitude(double a, double b, double c)
{
    return sqrt(a * a + b * b + c * c);
}

point crossProduct(point a, point b)
{
    point result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

void rotate_vectors(point *a, point *b, float angle)
{
    point temp;
    temp.x = a->x * cos(angle) + b->x * sin(angle);
    temp.y = a->y * cos(angle) + b->y * sin(angle);
    temp.z = a->z * cos(angle) + b->z * sin(angle);

    b->x = b->x * cos(angle) - a->x * sin(angle);
    b->y = b->y * cos(angle) - a->y * sin(angle);
    b->z = b->z * cos(angle) - a->z * sin(angle);

    *a = temp;
    return;
}

point rotate_vector(point v, point axis, double angle)
{
    normalize(&axis);

    point v_rot;
    double cos_theta = cos(angle);
    double sin_theta = sin(angle);

    point axis_cross_v = crossProduct(axis, v);

    double axis_dot_v = axis.x * v.x + axis.y * v.y + axis.z * v.z;

    v_rot.x = v.x * cos_theta +
              axis_cross_v.x * sin_theta +
              axis.x * axis_dot_v * (1 - cos_theta);

    v_rot.y = v.y * cos_theta +
              axis_cross_v.y * sin_theta +
              axis.y * axis_dot_v * (1 - cos_theta);

    v_rot.z = v.z * cos_theta +
              axis_cross_v.z * sin_theta +
              axis.z * axis_dot_v * (1 - cos_theta);

    return v_rot;
}

// Draw functions
void drawAxes()
{
    if (draw_axes == 1)
    {
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        {
            glVertex3f(100, 0, 0);
            glVertex3f(-100, 0, 0);

            // glVertex3f(0, 100, 0);
            // glVertex3f(0, -100, 0);

            glVertex3f(0, 0, 100);
            glVertex3f(0, 0, -100);
        }
        glEnd();
    }
}

void drawGrid()
{
    int i;
    if (draw_grid == 1)
    {
        glColor3f(0.6, 0.6, 0.6); // grey
        glBegin(GL_LINES);
        {
            for (i = -8; i <= 8; i++)
            {
                if (i == 0)
                    continue;

                glVertex3f(i * 10, -90, 0);
                glVertex3f(i * 10, 90, 0);

                glVertex3f(-90, i * 10, 0);
                glVertex3f(90, i * 10, 0);
            }
        }
        glEnd();
    }
}

void drawFloor()
{
    int i, j;
    double half_cube_size = CUBE_SIZE / 2;
    int tile_size_options[] = {10, 5};
    int tile_size = 5;

    for (int k = 0; k < 3; ++k)
    {
        if (fmod(half_cube_size, tile_size_options[k]) == 0)
        {
            tile_size = tile_size_options[k];
            break;
        }
    }

    int number_of_tiles = (int)(CUBE_SIZE / tile_size);

    glBegin(GL_QUADS);
    for (i = -number_of_tiles / 2; i < number_of_tiles / 2; i++)
    {
        for (j = -number_of_tiles / 2; j < number_of_tiles / 2; j++)
        {
            if (i * tile_size + tile_size > half_cube_size || i * tile_size < -half_cube_size ||
                j * tile_size + tile_size > half_cube_size || j * tile_size < -half_cube_size)
            {
                continue;
            }

            if ((i + j) % 2 == 0)
            {
                glColor3f(1.0, 1.0, 1.0);
            }
            else
            {
                glColor3f(0.1, 0.1, 0.1);
            }

            glVertex3f(i * tile_size, j * tile_size, 0);
            glVertex3f(i * tile_size, (j + 1) * tile_size, 0);
            glVertex3f((i + 1) * tile_size, (j + 1) * tile_size, 0);
            glVertex3f((i + 1) * tile_size, j * tile_size, 0);
        }
    }
    glEnd();
}

void drawCube(double a, double b)
{
    // double a = CUBE_SIZE / 2;
    glBegin(GL_QUADS);
    {
        glColor3f(0.5647f, 0.4353f, 0.8392f); // Purple
        // Top
        glVertex3f(a, a, b);
        glVertex3f(a, -a, b);
        glVertex3f(-a, -a, b);
        glVertex3f(-a, a, b);

        glColor3f(0.5647f, 0.4353f, 0.8392f); // Purple
        // Bottom -> Covered by floor
        glVertex3f(a, a, -b);
        glVertex3f(a, -a, -b);
        glVertex3f(-a, -a, -b);
        glVertex3f(-a, a, -b);

        glColor3f(0.0745f, 0.8f, 0.8392f); // Cyan
        // Front
        glVertex3f(a, a, b);
        glVertex3f(a, -a, b);
        glVertex3f(a, -a, -b);
        glVertex3f(a, a, -b);

        glColor3f(0.8196f, 0.7725f, 0.247f); // Yellow
        // Back
        glVertex3f(-a, a, b);
        glVertex3f(-a, -a, b);
        glVertex3f(-a, -a, -b);
        glVertex3f(-a, a, -b);

        glColor3f(0.247f, 0.8196f, 0.3216f); // Green
        // Right
        glVertex3f(a, a, b);
        glVertex3f(a, a, -b);
        glVertex3f(-a, a, -b);
        glVertex3f(-a, a, b);

        glColor3f(0.7882f, 0.3176f, 0.2784f); // Red
        // Left
        glVertex3f(a, -a, b);
        glVertex3f(a, -a, -b);
        glVertex3f(-a, -a, -b);
        glVertex3f(-a, -a, b);
    }
    glEnd();
}

void drawBall(double radius, int slices, int stacks)
{
    glPushMatrix();
    glRotatef(ball_rotation_angle * 180.0 / PI,
              ball_rotation_axis.x,
              ball_rotation_axis.y,
              ball_rotation_axis.z);

    for (int j = 0; j < slices; j++)
    {
        double lng0 = 2 * PI * (double)(j) / slices;
        double lng1 = 2 * PI * (double)(j + 1) / slices;

        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= stacks / 2; i++)
        {
            double lat = PI * (-0.5 + (double)i / stacks);
            double z = sin(lat);
            double r = cos(lat);

            double x0 = cos(lng0);
            double y0 = sin(lng0);
            double x1 = cos(lng1);
            double y1 = sin(lng1);

            if (j % 2 == 0)
                glColor3f(0.9, 0.1, 0.1);
            else
                glColor3f(0.1, 0.9, 0.1);

            glVertex3f(x0 * r * radius, y0 * r * radius, z * radius);
            glVertex3f(x1 * r * radius, y1 * r * radius, z * radius);
        }
        glEnd();

        glBegin(GL_QUAD_STRIP);
        for (int i = stacks / 2; i <= stacks; i++)
        {
            double lat = PI * (-0.5 + (double)i / stacks);
            double z = sin(lat);
            double r = cos(lat);

            double x0 = cos(lng0);
            double y0 = sin(lng0);
            double x1 = cos(lng1);
            double y1 = sin(lng1);

            if (j % 2 == 0)
                glColor3f(0.1, 0.9, 0.1);
            else
                glColor3f(0.9, 0.1, 0.1);

            glVertex3f(x0 * r * radius, y0 * r * radius, z * radius);
            glVertex3f(x1 * r * radius, y1 * r * radius, z * radius);
        }
        glEnd();
    }

    glPopMatrix();
}

void drawVelocityArrow()
{
    if (!show_velocity_arrow || magnitude(ball_velocity) < MIN_VELOCITY)
    {
        return;
    }

    point normal_velocity = ball_velocity;
    normalize(&normal_velocity);

    double arrowLength = BALL_RADIUS * ARROW_SCALE;

    // arrow line
    glColor3f(1.0, 1.0, 0.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(ball_position.x, ball_position.y, ball_position.z);
    glVertex3f(ball_position.x + normal_velocity.x * arrowLength,
               ball_position.y + normal_velocity.y * arrowLength,
               ball_position.z + normal_velocity.z * arrowLength);
    glEnd();
    glLineWidth(1.0);

    // arrow head
    glPushMatrix();
    glTranslatef(ball_position.x + normal_velocity.x * arrowLength,
                 ball_position.y + normal_velocity.y * arrowLength,
                 ball_position.z + normal_velocity.z * arrowLength);

    point up = {0, 0, 1};
    double arrow_angle = acos(normal_velocity.z) * 180.0 / PI;
    point rotation_axix = crossProduct(up, normal_velocity);
    if (magnitude(rotation_axix) < 0.001)
    {
        rotation_axix.x = 1;
        rotation_axix.y = 0;
        rotation_axix.z = 0;
    }
    normalize(&rotation_axix);

    glRotatef(arrow_angle, rotation_axix.x, rotation_axix.y, rotation_axix.z);

    GLUquadric *quadric = gluNewQuadric();
    gluCylinder(quadric, BALL_RADIUS * 0.3, 0, BALL_RADIUS, 5, 1);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

void resetBall()
{
    int half_cube_size = CUBE_SIZE / 2;
    int quarter_cube_size = half_cube_size / 2;

    ball_position.x = (double)(rand() % half_cube_size - quarter_cube_size);
    ball_position.y = (double)(rand() % half_cube_size - quarter_cube_size);
    ball_position.z = BALL_RADIUS;

    double theta = (double)(rand() % 360) * PI / 180.0;
    double phi = (double)(rand() % 180) * PI / 180.0;

    ball_velocity.x = initial_speed * sin(phi) * cos(theta);
    ball_velocity.y = initial_speed * sin(phi) * sin(theta);
    ball_velocity.z = initial_speed * cos(phi);

    ball_rotation_angle = 0;
    ball_rotation_axis.x = 1;
    ball_rotation_axis.y = 0;
    ball_rotation_axis.z = 0;

    simulation_running = 0;
}

void handleBallPhysics(double dt)
{
    if (!simulation_running)
    {
        return;
    }

    point prev_ball_position = ball_position;

    ball_velocity.z -= GRAVITY * dt;

    ball_position.x += ball_velocity.x * dt;
    ball_position.y += ball_velocity.y * dt;
    ball_position.z += ball_velocity.z * dt;

    double halfCube = CUBE_SIZE / 2;

    // Collision with walls
    if (ball_position.x - BALL_RADIUS < -halfCube)
    {
        ball_position.x = -halfCube + BALL_RADIUS;
        ball_velocity.x = -ball_velocity.x * RESTITUTION;
    }
    if (ball_position.x + BALL_RADIUS > halfCube)
    {
        ball_position.x = halfCube - BALL_RADIUS;
        ball_velocity.x = -ball_velocity.x * RESTITUTION;
    }

    if (ball_position.y - BALL_RADIUS < -halfCube)
    {
        ball_position.y = -halfCube + BALL_RADIUS;
        ball_velocity.y = -ball_velocity.y * RESTITUTION;
    }
    if (ball_position.y + BALL_RADIUS > halfCube)
    {
        ball_position.y = halfCube - BALL_RADIUS;
        ball_velocity.y = -ball_velocity.y * RESTITUTION;
    }

    // Collision with floor
    if (ball_position.z - BALL_RADIUS < 0)
    {
        ball_position.z = BALL_RADIUS;
        if (fabs(ball_velocity.z) > MIN_VELOCITY)
        {
            ball_velocity.z = -ball_velocity.z * RESTITUTION;
        }
        else
        {
            ball_velocity.z = 0;
        }
    }

    // Collision with ceiling
    if (ball_position.z + BALL_RADIUS > halfCube)
    {
        ball_position.z = halfCube - BALL_RADIUS;
        ball_velocity.z = -ball_velocity.z * RESTITUTION;
    }

    // rolling rotation
    if (magnitude(ball_velocity) > 0.01)
    {
        point displacement;
        displacement.x = ball_position.x - prev_ball_position.x;
        displacement.y = ball_position.y - prev_ball_position.y;
        displacement.z = ball_position.z - prev_ball_position.z;

        if (magnitude(displacement) > 0.001)
        {
            point velXY = {ball_velocity.x, ball_velocity.y, 0};
            normalize(&velXY);

            point up = {0, 0, 1};
            ball_rotation_axis = crossProduct(velXY, up);
            normalize(&ball_rotation_axis);

            double displacement2D = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            ball_rotation_angle += displacement2D / BALL_RADIUS;
        }
    }

    // the ball stops if it's moving too slowly while on the floor
    if (ball_position.z <= BALL_RADIUS + 0.01 && magnitude(ball_velocity) < MIN_VELOCITY)
    {
        ball_velocity.x = 0;
        ball_velocity.y = 0;
        ball_velocity.z = 0;
    }
}

void drawBlade()
{
    glBegin(GL_TRIANGLES);
    glColor3f(0.53f, 0.81f, 0.92f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 30, 40);
    glVertex3f(10, 30, 40);
    glEnd();
}

void drawSS()
{
    glPushMatrix();
    glColor3f(0, 1, 0);
    drawCube(10, 40);
    // drawFloor();

    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(-30, 0, 10);
    drawCube(2, 30);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(40, 0, 30);
    glRotatef(bladeang, 1, 0, 0);

    glColor3f(0, 0, 1);

    drawBlade();

    glPushMatrix();
    glRotatef(120, 1.0, 0.0, 0.0);
    drawBlade();
    glPopMatrix();

    glPushMatrix();
    glRotatef(240, 1.0, 0.0, 0.0);
    drawBlade();
    glPopMatrix();

    glPopMatrix();

    glPopMatrix();
}

void keyboardListener(unsigned char key, int x, int y)
{
    double rate = 0.05;
    double wo_ref_rate = 0.01;
    double rotation_speed = 0.02;
    double prev_dist = 0;
    double current_dist = 0;
    point temp;
    double rotation_angle = 0;
    point direction;

    switch (key)
    {
    case '1': // yaw right
        look_direction_vector = rotate_vector(look_direction_vector, up_vector, rotation_speed);
        normalize(&look_direction_vector);
        right_vector = rotate_vector(right_vector, up_vector, rotation_speed);
        normalize(&right_vector);
        break;

    case '2': // yaw left
        look_direction_vector = rotate_vector(look_direction_vector, up_vector, -rotation_speed);
        normalize(&look_direction_vector);
        right_vector = rotate_vector(right_vector, up_vector, -rotation_speed);
        normalize(&right_vector);
        break;

    case '3': // pitch up
        look_direction_vector = rotate_vector(look_direction_vector, right_vector, rotation_speed);
        normalize(&look_direction_vector);
        up_vector = rotate_vector(up_vector, right_vector, rotation_speed);
        normalize(&up_vector);
        break;

    case '4': // pitch down
        look_direction_vector = rotate_vector(look_direction_vector, right_vector, -rotation_speed);
        normalize(&look_direction_vector);
        up_vector = rotate_vector(up_vector, right_vector, -rotation_speed);
        normalize(&up_vector);
        break;

    case '5': // tilt clockwise
        right_vector = rotate_vector(right_vector, look_direction_vector, rotation_speed);
        normalize(&right_vector);
        up_vector = rotate_vector(up_vector, look_direction_vector, rotation_speed);
        normalize(&up_vector);
        break;

    case '6': // tilt counter-clockwise
        right_vector = rotate_vector(right_vector, look_direction_vector, -rotation_speed);
        normalize(&right_vector);
        up_vector = rotate_vector(up_vector, look_direction_vector, -rotation_speed);
        normalize(&up_vector);
        break;

    case 'w': // Move upward without changing reference point
        bladespeed += 2;
        if (bladespeed >= 10)
        {
            bladespeed = 10;
        }
        break;

    case 's': // Move downward without changing reference point
        if (bladespeed <= 2)
        {
            bladespeed += 2;
        }
        bladespeed -= 2;
        break;

    case ' ':
        simulation_running = 1 - simulation_running;
        break;

    case 'r':
        resetBall();
        break;

    case 'v':
        show_velocity_arrow = 1 - show_velocity_arrow;
        break;

    case 'd':
        draw_grid = 1 - draw_grid;
        break;

    case 'a':
        draw_axes = 1 - draw_axes;
        break;

    case '+':
        if (!simulation_running)
        {
            initial_speed += 5.0;
            if (initial_speed > 100.0)
                initial_speed = 100.0;

            direction = ball_velocity;
            normalize(&direction);
            ball_velocity.x = direction.x * initial_speed;
            ball_velocity.y = direction.y * initial_speed;
            ball_velocity.z = direction.z * initial_speed;
        }
        break;

    case '-':
        if (!simulation_running)
        {
            initial_speed -= 5.0;
            if (initial_speed < 5.0)
                initial_speed = 5.0;

            direction = ball_velocity;
            normalize(&direction);
            ball_velocity.x = direction.x * initial_speed;
            ball_velocity.y = direction.y * initial_speed;
            ball_velocity.z = direction.z * initial_speed;
        }
        break;
    }
}

void specialKeyListener(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        camera_position.x += look_direction_vector.x * 2;
        camera_position.y += look_direction_vector.y * 2;
        camera_position.z += look_direction_vector.z * 2;
        break;
    case GLUT_KEY_DOWN:
        camera_position.x -= look_direction_vector.x * 2;
        camera_position.y -= look_direction_vector.y * 2;
        camera_position.z -= look_direction_vector.z * 2;
        break;
    case GLUT_KEY_RIGHT:
        camera_position.x += right_vector.x * 2;
        camera_position.y += right_vector.y * 2;
        camera_position.z += right_vector.z * 2;
        break;
    case GLUT_KEY_LEFT:
        camera_position.x -= right_vector.x * 2;
        camera_position.y -= right_vector.y * 2;
        camera_position.z -= right_vector.z * 2;
        break;
    case GLUT_KEY_PAGE_UP:
        camera_position.x += up_vector.x * 2;
        camera_position.y += up_vector.y * 2;
        camera_position.z += up_vector.z * 2;
        break;
    case GLUT_KEY_PAGE_DOWN:
        camera_position.x -= up_vector.x * 2;
        camera_position.y -= up_vector.y * 2;
        camera_position.z -= up_vector.z * 2;
        break;
    }
}

void mouseListener(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
            draw_axes = 1 - draw_axes;
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
            draw_grid = 1 - draw_grid;
        break;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camera_position.x, camera_position.y, camera_position.z,
              camera_position.x + look_direction_vector.x, camera_position.y + look_direction_vector.y, camera_position.z + look_direction_vector.z,
              up_vector.x, up_vector.y, up_vector.z);

    glMatrixMode(GL_MODELVIEW);

    drawAxes();
    drawGrid();
    drawSS();

    glutSwapBuffers();
}

void animate()
{
    animation_angle += 0.05;
    bladeang += bladespeed;

    static double lastTime = 0;
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    double deltaTime = currentTime - lastTime;

    if (deltaTime > 0.016)
    {
        handleBallPhysics(deltaTime);
        lastTime = currentTime;
    }

    glutPostRedisplay();
}

void timer(int value)
{
    bladeang += bladespeed;
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
void init()
{
    draw_grid = 0;
    draw_axes = 0;
    animation_angle = 0;

    normalize(&look_direction_vector);
    normalize(&right_vector);
    normalize(&up_vector);

    right_vector = crossProduct(up_vector, look_direction_vector);
    normalize(&right_vector);

    up_vector = crossProduct(look_direction_vector, right_vector);
    normalize(&up_vector);

    resetBall();

    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80, 1, 1, 1000.0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

    glutCreateWindow("OpenGL - Camera and Ball Controls");

    init();
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    // glutIdleFunc(animate);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutMouseFunc(mouseListener);

    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}