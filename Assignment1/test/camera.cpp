#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define PI (2 * acos(0.0))
#define CUBE_SIZE 120
#define BALL_RADIUS 5.0
#define NUMBER_OF_BALL_STRIPES 30
#define GRAVITY 9.8
#define RESTITUTION 0.75
#define MIN_VELOCITY 0.1
#define ARROW_SCALE 0.5

struct point
{
    double x, y, z;
};

// Variables: Camera & directions
point camera_position = {100, 100, 100};
point look_direction_vector = {-1, -1, -1};
point right_vector = {1, -1, 0};
point up_vector = {0, 0, 1};

// Variables: Environment
double angle = 0;

// Variables: Cube
point cube_center = {0, 0, 0};

// Variables: Ball position and velocity
point ball_position = {0, 0, BALL_RADIUS};
point ball_velocity = {0, 0, 0};
point initial_velocity = {0, 0, 40};
double ball_rotation_angle = 0;
point ball_rotation_axix = {1, 0, 0};
double initial_speed = 40.0;

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

            glVertex3f(0, 100, 0);
            glVertex3f(0, -100, 0);

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
    int tileSize = 15;
    double half_cube_size = CUBE_SIZE / 2;
    int numTiles = (int)(CUBE_SIZE / tileSize);

    glBegin(GL_QUADS);
    for (i = -numTiles / 2; i < numTiles / 2; i++)
    {
        for (j = -numTiles / 2; j < numTiles / 2; j++)
        {
            // Make sure tiles stay within cube bounds
            if (i * tileSize + tileSize > half_cube_size || i * tileSize < -half_cube_size ||
                j * tileSize + tileSize > half_cube_size || j * tileSize < -half_cube_size)
            {
                continue;
            }

            if ((i + j) % 2 == 0)
            {
                glColor3f(1.0, 1.0, 1.0); // White
            }
            else
            {
                glColor3f(0.0, 0.0, 0.0); // Black
            }

            // double floor_z_coordinate = -1 * half_cube_size;
            double floor_z_coordinate = 0;

            glVertex3f(i * tileSize, j * tileSize, floor_z_coordinate);
            glVertex3f(i * tileSize, (j + 1) * tileSize, floor_z_coordinate);
            glVertex3f((i + 1) * tileSize, (j + 1) * tileSize, floor_z_coordinate);
            glVertex3f((i + 1) * tileSize, j * tileSize, floor_z_coordinate);
        }
    }
    glEnd();
}

void drawCube()
{
    double a = CUBE_SIZE / 2;
    glBegin(GL_QUADS);
    {
        glColor3f(1, 0, 0);
        // Top
        glVertex3f(a, a, a);
        glVertex3f(a, -a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, a, a);

        // glColor3f(1, 0, 0);
        // Bottom -> Covered by floor
        // glVertex3f(a, a, 0);
        // glVertex3f(a, -a, 0);
        // glVertex3f(-a, -a, 0);
        // glVertex3f(-a, a, 0);

        glColor3f(0, 1, 0);
        // Front
        glVertex3f(a, a, a);
        glVertex3f(a, -a, a);
        glVertex3f(a, -a, 0);
        glVertex3f(a, a, 0);

        glColor3f(0, 1, 0);
        // Back
        glVertex3f(-a, a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, -a, 0);
        glVertex3f(-a, a, 0);

        glColor3f(0, 0, 1);
        // Right
        glVertex3f(a, a, a);
        glVertex3f(-a, a, a);
        glVertex3f(-a, a, 0);
        glVertex3f(a, a, 0);

        glColor3f(0, 0, 1);
        // Left
        glVertex3f(a, -a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, -a, 0);
        glVertex3f(a, -a, 0);
    }
    glEnd();
}

void drawBall(double radius, int slices, int stacks)
{
    glPushMatrix();
    glRotatef(ball_rotation_angle * 180.0 / PI,
              ball_rotation_axix.x,
              ball_rotation_axix.y,
              ball_rotation_axix.z);

    // Loop over vertical slices (longitude)
    for (int j = 0; j < slices; j++)
    {
        double lng0 = 2 * PI * (double)(j) / slices;
        double lng1 = 2 * PI * (double)(j + 1) / slices;

        // Split into upper and lower hemispheres
        // ----- UPPER HALF -----
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= stacks / 2; i++) // From equator up to top
        {
            double lat = PI * (-0.5 + (double)i / stacks);
            double z = sin(lat);
            double r = cos(lat);

            double x0 = cos(lng0);
            double y0 = sin(lng0);
            double x1 = cos(lng1);
            double y1 = sin(lng1);

            // Coloring upper half
            if (j % 2 == 0)
                glColor3f(0.9, 0.1, 0.1); // Red
            else
                glColor3f(0.1, 0.9, 0.1); // Green

            glVertex3f(x0 * r * radius, y0 * r * radius, z * radius);
            glVertex3f(x1 * r * radius, y1 * r * radius, z * radius);
        }
        glEnd();

        // ----- LOWER HALF -----
        glBegin(GL_QUAD_STRIP);
        for (int i = stacks / 2; i <= stacks; i++) // From equator down to bottom
        {
            double lat = PI * (-0.5 + (double)i / stacks);
            double z = sin(lat);
            double r = cos(lat);

            double x0 = cos(lng0);
            double y0 = sin(lng0);
            double x1 = cos(lng1);
            double y1 = sin(lng1);

            // Coloring lower half
            if (j % 2 == 0)
                glColor3f(0.1, 0.9, 0.1); // Green
            else
                glColor3f(0.9, 0.1, 0.1); // Red

            glVertex3f(x0 * r * radius, y0 * r * radius, z * radius);
            glVertex3f(x1 * r * radius, y1 * r * radius, z * radius);
        }
        glEnd();
    }

    glPopMatrix();
}

void drawVelocityArrow()
{
    if (!show_velocity_arrow || magnitude(ball_velocity) < 0.1)
    {
        return;
    }

    point normal_velocity = ball_velocity;
    normalize(&normal_velocity);

    double arrowLength = BALL_RADIUS * 2 * ARROW_SCALE * magnitude(ball_velocity) / initial_speed;

    // Draw arrow line
    glColor3f(1.0, 1.0, 0.0); // Yellow
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(ball_position.x, ball_position.y, ball_position.z);
    glVertex3f(ball_position.x + normal_velocity.x * arrowLength,
               ball_position.y + normal_velocity.y * arrowLength,
               ball_position.z + normal_velocity.z * arrowLength);
    glEnd();
    glLineWidth(1.0);

    // Draw arrow head
    glPushMatrix();
    glTranslatef(ball_position.x + normal_velocity.x * arrowLength,
                 ball_position.y + normal_velocity.y * arrowLength,
                 ball_position.z + normal_velocity.z * arrowLength);

    // Rotate to point in velocity direction
    point up = {0, 0, 1};
    double angle = acos(normal_velocity.z) * 180.0 / PI;
    point rotation_axix = crossProduct(up, normal_velocity);
    if (magnitude(rotation_axix) < 0.001)
    {
        rotation_axix.x = 1;
        rotation_axix.y = 0;
        rotation_axix.z = 0;
    }
    normalize(&rotation_axix);

    glRotatef(angle, rotation_axix.x, rotation_axix.y, rotation_axix.z);

    // Draw the cone
    GLUquadric *quadric = gluNewQuadric();
    gluCylinder(quadric, BALL_RADIUS * 0.3 * ARROW_SCALE, 0, BALL_RADIUS * ARROW_SCALE, 10, 1);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

// Reset ball with random position and velocity
void resetBall()
{
    int half_cube_size = CUBE_SIZE / 2;
    int quarter_cube_size = half_cube_size / 2;

    // Random position within the cube
    ball_position.x = (double)(rand() % half_cube_size - quarter_cube_size);
    ball_position.y = (double)(rand() % half_cube_size - quarter_cube_size);
    ball_position.z = BALL_RADIUS;

    // Random initial velocity in any direction
    double theta = (double)(rand() % 360) * PI / 180.0;
    double phi = (double)(rand() % 180) * PI / 180.0;

    ball_velocity.x = initial_speed * sin(phi) * cos(theta);
    ball_velocity.y = initial_speed * sin(phi) * sin(theta);
    ball_velocity.z = initial_speed * cos(phi);

    // No initial rotation/simulation
    ball_rotation_angle = 0;
    ball_rotation_axix.x = 1;
    ball_rotation_axix.y = 0;
    ball_rotation_axix.z = 0;

    simulation_running = 0;
}

void drawSS()
{
    glPushMatrix();
    glColor3f(0, 1, 0);
    drawCube();
    drawFloor();

    glPushMatrix();
    glTranslatef(ball_position.x, ball_position.y, ball_position.z);
    drawBall(BALL_RADIUS, NUMBER_OF_BALL_STRIPES, NUMBER_OF_BALL_STRIPES);
    glPopMatrix();

    drawVelocityArrow();

    glPopMatrix();
}

// Function to adjust camera to always look at the cube
void adjustCameraToLookAtCube()
{
    point direction;
    direction.x = cube_center.x - camera_position.x;
    direction.y = cube_center.y - camera_position.y;
    direction.z = cube_center.z - camera_position.z;

    look_direction_vector = direction;
    normalize(&look_direction_vector);

    // No need adjust the right and up vectors
    // Recalculate right vector (assuming world up is (0,0,1))
    // point world_up = {0, 0, 1};
    // right_vector.x = direction.y * world_up.z - direction.z * world_up.y;
    // right_vector.y = direction.z * world_up.x - direction.x * world_up.z;
    // right_vector.z = direction.x * world_up.y - direction.y * world_up.x;
    // normalize(&right_vector);

    // // Recalculate up vector to ensure orthogonality
    // up_vector.x = look_direction_vector.y * right_vector.z - look_direction_vector.z * right_vector.y;
    // up_vector.y = look_direction_vector.z * right_vector.x - look_direction_vector.x * right_vector.z;
    // up_vector.z = look_direction_vector.x * right_vector.y - look_direction_vector.y * right_vector.x;
    // normalize(&up_vector);
}

void keyboardListener(unsigned char key, int x, int y)
{
    double rate = 0.05;
    point temp;

    switch (key)
    {
    case '1': // yaw right
        rotate_vectors(&look_direction_vector, &right_vector, (-1 * rate));

        normalize(&look_direction_vector);
        normalize(&right_vector);

        up_vector = crossProduct(look_direction_vector, right_vector);
        normalize(&up_vector);
        break;

    case '2': // yaw left
        rotate_vectors(&look_direction_vector, &right_vector, rate);

        normalize(&look_direction_vector);
        normalize(&right_vector);

        up_vector = crossProduct(look_direction_vector, right_vector);
        normalize(&up_vector);
        break;

    case '3': // pitch up
        rotate_vectors(&look_direction_vector, &up_vector, rate);

        normalize(&look_direction_vector);
        normalize(&up_vector);

        right_vector = crossProduct(up_vector, look_direction_vector);
        normalize(&right_vector);
        break;

    case '4': // pitch down
        rotate_vectors(&look_direction_vector, &up_vector, (-1 * rate));

        normalize(&look_direction_vector);
        normalize(&up_vector);

        right_vector = crossProduct(up_vector, look_direction_vector);
        normalize(&right_vector);
        break;

    case '5': // roll clockwise
        rotate_vectors(&right_vector, &up_vector, rate);

        normalize(&right_vector);
        normalize(&up_vector);

        look_direction_vector = crossProduct(right_vector, up_vector);
        normalize(&look_direction_vector);
        break;

    case '6': // roll counter-clockwise
        rotate_vectors(&right_vector, &up_vector, (-1 * rate));

        normalize(&right_vector);
        normalize(&up_vector);

        look_direction_vector = crossProduct(right_vector, up_vector);
        normalize(&look_direction_vector);
        break;

    case 'w':
        camera_position.z += up_vector.z * 2;
        adjustCameraToLookAtCube();
        break;

    case 's':
        camera_position.z -= up_vector.z * 2;
        adjustCameraToLookAtCube();
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
    angle += 0.05;
    glutPostRedisplay();
}

void init()
{
    draw_grid = 0;
    draw_axes = 0;
    angle = 0;

    // Normalize the direction vectors
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
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

    glutCreateWindow("Camera Controls - OpenGL");

    init();
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutIdleFunc(animate);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutMouseFunc(mouseListener);

    glutMainLoop();
    return 0;
}