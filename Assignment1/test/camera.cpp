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
point ballPos = {0, 0, BALL_RADIUS};
point ballVel = {0, 0, 0};
point initialVel = {0, 0, 40};
double ballRotationAngle = 0;
point ballRotationAxis = {1, 0, 0};
double initialSpeed = 40.0;

// Variables: simulation
int drawgrid = 0;
int drawaxes = 0;
int simRunning = 0;
int showVelocityArrow = 1;

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

// Draw functions
void drawAxes()
{
    if (drawaxes == 1)
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
    if (drawgrid == 1)
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
    double halfCube = CUBE_SIZE / 2;
    int numTiles = (int)(CUBE_SIZE / tileSize);

    glBegin(GL_QUADS);
    for (i = -numTiles / 2; i < numTiles / 2; i++)
    {
        for (j = -numTiles / 2; j < numTiles / 2; j++)
        {
            // Make sure tiles stay within cube bounds
            if (i * tileSize + tileSize > halfCube || i * tileSize < -halfCube ||
                j * tileSize + tileSize > halfCube || j * tileSize < -halfCube)
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

            // double floor_z_coordinate = -1 * halfCube;
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
    glRotatef(ballRotationAngle * 180.0 / PI,
              ballRotationAxis.x,
              ballRotationAxis.y,
              ballRotationAxis.z);

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

void drawSS()
{
    glPushMatrix();
    glColor3f(0, 1, 0);
    // glTranslatef(0, 0, 0);
    // glRotatef(0, 1, 1, 1);
    drawCube();
    drawFloor();

    glPushMatrix();
    glTranslatef(ballPos.x, ballPos.y, ballPos.z);
    drawBall(BALL_RADIUS, NUMBER_OF_BALL_STRIPES, NUMBER_OF_BALL_STRIPES);
    glPopMatrix();

    glPopMatrix();
}

// Function to adjust camera to always look at the cube
void adjustCameraToLookAtCube()
{
    point direction;
    direction.x = cube_center.x - camera_position.x;
    direction.y = cube_center.y - camera_position.y;
    direction.z = cube_center.z - camera_position.z;

    // double len = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    // direction.x /= len;
    // direction.y /= len;
    // direction.z /= len;

    // Set look_direction_vector to normalized direction
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
        temp.x = look_direction_vector.x * cos(-rate) + right_vector.x * sin(-rate);
        temp.y = look_direction_vector.y * cos(-rate) + right_vector.y * sin(-rate);
        temp.z = look_direction_vector.z * cos(-rate) + right_vector.z * sin(-rate);
        right_vector.x = right_vector.x * cos(-rate) - look_direction_vector.x * sin(-rate);
        right_vector.y = right_vector.y * cos(-rate) - look_direction_vector.y * sin(-rate);
        right_vector.z = right_vector.z * cos(-rate) - look_direction_vector.z * sin(-rate);
        look_direction_vector = temp;

        // Normalize vectors
        normalize(&look_direction_vector);
        normalize(&right_vector);

        up_vector = crossProduct(look_direction_vector, right_vector);
        normalize(&up_vector);
        break;

    case '2': // yaw left
        temp.x = look_direction_vector.x * cos(rate) + right_vector.x * sin(rate);
        temp.y = look_direction_vector.y * cos(rate) + right_vector.y * sin(rate);
        temp.z = look_direction_vector.z * cos(rate) + right_vector.z * sin(rate);
        right_vector.x = right_vector.x * cos(rate) - look_direction_vector.x * sin(rate);
        right_vector.y = right_vector.y * cos(rate) - look_direction_vector.y * sin(rate);
        right_vector.z = right_vector.z * cos(rate) - look_direction_vector.z * sin(rate);
        look_direction_vector = temp;

        // Normalize vectors
        normalize(&look_direction_vector);
        normalize(&right_vector);

        up_vector = crossProduct(look_direction_vector, right_vector);
        normalize(&up_vector);
        break;

    case '3': // pitch up
        temp.x = look_direction_vector.x * cos(rate) + up_vector.x * sin(rate);
        temp.y = look_direction_vector.y * cos(rate) + up_vector.y * sin(rate);
        temp.z = look_direction_vector.z * cos(rate) + up_vector.z * sin(rate);
        up_vector.x = up_vector.x * cos(rate) - look_direction_vector.x * sin(rate);
        up_vector.y = up_vector.y * cos(rate) - look_direction_vector.y * sin(rate);
        up_vector.z = up_vector.z * cos(rate) - look_direction_vector.z * sin(rate);
        look_direction_vector = temp;

        // Normalize vectors
        normalize(&look_direction_vector);
        normalize(&up_vector);

        right_vector = crossProduct(up_vector, look_direction_vector);
        normalize(&right_vector);
        break;

    case '4': // pitch down
        temp.x = look_direction_vector.x * cos(-rate) + up_vector.x * sin(-rate);
        temp.y = look_direction_vector.y * cos(-rate) + up_vector.y * sin(-rate);
        temp.z = look_direction_vector.z * cos(-rate) + up_vector.z * sin(-rate);
        up_vector.x = up_vector.x * cos(-rate) - look_direction_vector.x * sin(-rate);
        up_vector.y = up_vector.y * cos(-rate) - look_direction_vector.y * sin(-rate);
        up_vector.z = up_vector.z * cos(-rate) - look_direction_vector.z * sin(-rate);
        look_direction_vector = temp;

        // Normalize vectors
        normalize(&look_direction_vector);
        normalize(&up_vector);

        right_vector = crossProduct(up_vector, look_direction_vector);
        normalize(&right_vector);
        break;

    case '5': // roll clockwise
        temp.x = right_vector.x * cos(rate) + up_vector.x * sin(rate);
        temp.y = right_vector.y * cos(rate) + up_vector.y * sin(rate);
        temp.z = right_vector.z * cos(rate) + up_vector.z * sin(rate);
        up_vector.x = up_vector.x * cos(rate) - right_vector.x * sin(rate);
        up_vector.y = up_vector.y * cos(rate) - right_vector.y * sin(rate);
        up_vector.z = up_vector.z * cos(rate) - right_vector.z * sin(rate);
        right_vector = temp;

        // Normalize vectors
        normalize(&right_vector);
        normalize(&up_vector);

        look_direction_vector = crossProduct(right_vector, up_vector);
        normalize(&look_direction_vector);
        break;

    case '6': // roll counter-clockwise
        temp.x = right_vector.x * cos(-rate) + up_vector.x * sin(-rate);
        temp.y = right_vector.y * cos(-rate) + up_vector.y * sin(-rate);
        temp.z = right_vector.z * cos(-rate) + up_vector.z * sin(-rate);
        up_vector.x = up_vector.x * cos(-rate) - right_vector.x * sin(-rate);
        up_vector.y = up_vector.y * cos(-rate) - right_vector.y * sin(-rate);
        up_vector.z = up_vector.z * cos(-rate) - right_vector.z * sin(-rate);
        right_vector = temp;

        // Normalize vectors
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

    case 'd': // Toggle grid
        drawgrid = 1 - drawgrid;
        break;

    case 'a': // Toggle axes
        drawaxes = 1 - drawaxes;
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
            drawaxes = 1 - drawaxes;
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
            drawgrid = 1 - drawgrid;
        break;
    case GLUT_MIDDLE_BUTTON:
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
    drawgrid = 0;
    drawaxes = 0;
    angle = 0;

    // Normalize the direction vectors
    normalize(&look_direction_vector);
    normalize(&right_vector);
    normalize(&up_vector);

    right_vector = crossProduct(up_vector, look_direction_vector);
    normalize(&right_vector);

    up_vector = crossProduct(look_direction_vector, right_vector);
    normalize(&up_vector);

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