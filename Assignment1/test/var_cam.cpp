#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define pi (2 * acos(0.0))

struct point
{
    double x, y, z;
};

// Initial camera position and direction vectors (renamed for clarity)
point camera_position = {100, 100, 100};
point look_direction_vector = {-1, -1, -1}; // Look direction (toward origin)
point right_vector = {1, -1, 0};            // Right vector
point up_vector = {0, 0, 1};                // Up vector

// Cube position (center of the scene)
point cube_center = {0, 0, 0};
double cube_size = 60;

double angle = 0;
int drawgrid = 0;
int drawaxes = 0;

// Utility function to normalize a vector
void normalize(point *p)
{
    double len = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
    p->x /= len;
    p->y /= len;
    p->z /= len;
}

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

// Function to draw a checkered floor
void drawCheckeredFloor(double size, int divisions)
{
    double unit = size * 2 / divisions;
    int i, j;
    int isWhite;

    glBegin(GL_QUADS);
    for (i = 0; i < divisions; i++)
    {
        for (j = 0; j < divisions; j++)
        {
            // Alternate black and white
            isWhite = (i + j) % 2;

            if (isWhite)
                glColor3f(1.0, 1.0, 1.0); // White
            else
                glColor3f(0.0, 0.0, 0.0); // Black

            double x1 = -size + i * unit;
            double y1 = -size + j * unit;
            double x2 = x1 + unit;
            double y2 = y1 + unit;

            // Draw the square at z = -size (bottom face of cube)
            glVertex3f(x1, y1, -size);
            glVertex3f(x2, y1, -size);
            glVertex3f(x2, y2, -size);
            glVertex3f(x1, y2, -size);
        }
    }
    glEnd();
}

void drawCube(double a)
{
    glBegin(GL_QUADS);
    {
        glColor3f(1, 0, 0);
        // Front
        glVertex3f(a, a, a);
        glVertex3f(a, -a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, a, a);

        glColor3f(1, 0, 0);
        // Back
        glVertex3f(a, a, -a);
        glVertex3f(a, -a, -a);
        glVertex3f(-a, -a, -a);
        glVertex3f(-a, a, -a);

        glColor3f(0, 1, 0);
        // Right
        glVertex3f(a, a, a);
        glVertex3f(a, -a, a);
        glVertex3f(a, -a, -a);
        glVertex3f(a, a, -a);

        glColor3f(0, 1, 0);
        // Left
        glVertex3f(-a, a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, -a, -a);
        glVertex3f(-a, a, -a);

        glColor3f(0, 0, 1);
        // Top
        glVertex3f(a, a, a);
        glVertex3f(-a, a, a);
        glVertex3f(-a, a, -a);
        glVertex3f(a, a, -a);

        // Bottom face is handled by checkered floor
        // Don't draw the original bottom face
    }
    glEnd();

    // Draw checkered floor for the bottom face
    drawCheckeredFloor(a, 8); // 8x8 checkered pattern
}

void drawSS()
{
    glPushMatrix();
    glColor3f(0, 1, 0);
    // glTranslatef(0, 0, 0);
    // glRotatef(0, 1, 1, 1);
    drawCube(cube_size);
    glPopMatrix();
}

// Function to adjust camera to always look at the cube
void adjustCameraToLookAtCube()
{
    // Calculate direction vector from camera to cube
    point direction;
    direction.x = cube_center.x - camera_position.x;
    direction.y = cube_center.y - camera_position.y;
    direction.z = cube_center.z - camera_position.z;

    // Normalize this direction
    double len = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    direction.x /= len;
    direction.y /= len;
    direction.z /= len;

    // Set look_direction_vector to this direction
    look_direction_vector = direction;

    // Recalculate right vector (assuming world up is (0,0,1))
    point world_up = {0, 0, 1};
    right_vector.x = direction.y * world_up.z - direction.z * world_up.y;
    right_vector.y = direction.z * world_up.x - direction.x * world_up.z;
    right_vector.z = direction.x * world_up.y - direction.y * world_up.x;
    normalize(&right_vector);

    // Recalculate up vector to ensure orthogonality
    up_vector.x = look_direction_vector.y * right_vector.z - look_direction_vector.z * right_vector.y;
    up_vector.y = look_direction_vector.z * right_vector.x - look_direction_vector.x * right_vector.z;
    up_vector.z = look_direction_vector.x * right_vector.y - look_direction_vector.y * right_vector.x;
    normalize(&up_vector);
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

        // Recalculate up_vector to ensure orthogonality
        up_vector.x = look_direction_vector.y * right_vector.z - look_direction_vector.z * right_vector.y;
        up_vector.y = look_direction_vector.z * right_vector.x - look_direction_vector.x * right_vector.z;
        up_vector.z = look_direction_vector.x * right_vector.y - look_direction_vector.y * right_vector.x;
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

        // Recalculate up_vector to ensure orthogonality
        up_vector.x = look_direction_vector.y * right_vector.z - look_direction_vector.z * right_vector.y;
        up_vector.y = look_direction_vector.z * right_vector.x - look_direction_vector.x * right_vector.z;
        up_vector.z = look_direction_vector.x * right_vector.y - look_direction_vector.y * right_vector.x;
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

        // Recalculate right_vector to ensure orthogonality
        right_vector.x = up_vector.y * look_direction_vector.z - up_vector.z * look_direction_vector.y;
        right_vector.y = up_vector.z * look_direction_vector.x - up_vector.x * look_direction_vector.z;
        right_vector.z = up_vector.x * look_direction_vector.y - up_vector.y * look_direction_vector.x;
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

        // Recalculate right_vector to ensure orthogonality
        right_vector.x = up_vector.y * look_direction_vector.z - up_vector.z * look_direction_vector.y;
        right_vector.y = up_vector.z * look_direction_vector.x - up_vector.x * look_direction_vector.z;
        right_vector.z = up_vector.x * look_direction_vector.y - up_vector.y * look_direction_vector.x;
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

        // Recalculate look_direction_vector to ensure orthogonality
        look_direction_vector.x = right_vector.y * up_vector.z - right_vector.z * up_vector.y;
        look_direction_vector.y = right_vector.z * up_vector.x - right_vector.x * up_vector.z;
        look_direction_vector.z = right_vector.x * up_vector.y - right_vector.y * up_vector.x;
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

        // Recalculate look_direction_vector to ensure orthogonality
        look_direction_vector.x = right_vector.y * up_vector.z - right_vector.z * up_vector.y;
        look_direction_vector.y = right_vector.z * up_vector.x - right_vector.x * up_vector.z;
        look_direction_vector.z = right_vector.x * up_vector.y - right_vector.y * up_vector.x;
        normalize(&look_direction_vector);
        break;

    case 'w': // Move camera up while looking at cube
        camera_position.z += 2;
        adjustCameraToLookAtCube();
        break;

    case 's': // Move camera down while looking at cube
        camera_position.z -= 2;
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
        adjustCameraToLookAtCube();
        break;
    case GLUT_KEY_DOWN:
        camera_position.x -= look_direction_vector.x * 2;
        camera_position.y -= look_direction_vector.y * 2;
        camera_position.z -= look_direction_vector.z * 2;
        adjustCameraToLookAtCube();
        break;
    case GLUT_KEY_RIGHT:
        camera_position.x += right_vector.x * 2;
        camera_position.y += right_vector.y * 2;
        camera_position.z += right_vector.z * 2;
        adjustCameraToLookAtCube();
        break;
    case GLUT_KEY_LEFT:
        camera_position.x -= right_vector.x * 2;
        camera_position.y -= right_vector.y * 2;
        camera_position.z -= right_vector.z * 2;
        adjustCameraToLookAtCube();
        break;
    case GLUT_KEY_PAGE_UP:
        camera_position.x += up_vector.x * 2;
        camera_position.y += up_vector.y * 2;
        camera_position.z += up_vector.z * 2;
        adjustCameraToLookAtCube();
        break;
    case GLUT_KEY_PAGE_DOWN:
        camera_position.x -= up_vector.x * 2;
        camera_position.y -= up_vector.y * 2;
        camera_position.z -= up_vector.z * 2;
        adjustCameraToLookAtCube();
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
              camera_position.x + look_direction_vector.x,
              camera_position.y + look_direction_vector.y,
              camera_position.z + look_direction_vector.z,
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

    // Make sure the vectors are orthogonal
    // First, make sure right is perpendicular to look
    // This is the cross product of world-up and look
    right_vector.x = up_vector.y * look_direction_vector.z - up_vector.z * look_direction_vector.y;
    right_vector.y = up_vector.z * look_direction_vector.x - up_vector.x * look_direction_vector.z;
    right_vector.z = up_vector.x * look_direction_vector.y - up_vector.y * look_direction_vector.x;
    normalize(&right_vector);

    // Then recompute up as cross product of look and right
    up_vector.x = look_direction_vector.y * right_vector.z - look_direction_vector.z * right_vector.y;
    up_vector.y = look_direction_vector.z * right_vector.x - look_direction_vector.x * right_vector.z;
    up_vector.z = look_direction_vector.x * right_vector.y - look_direction_vector.y * right_vector.x;
    normalize(&up_vector);

    // Initialize camera to look at cube
    adjustCameraToLookAtCube();

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

    glutCreateWindow("Enhanced Camera Controls - OpenGL");

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