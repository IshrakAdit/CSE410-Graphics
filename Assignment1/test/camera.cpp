#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define pi (2 * acos(0.0))

struct point
{
    double x, y, z;
};

// Initial camera position and direction vectors
point pos = {100, 100, 100};
point l = {-1, -1, -1}; // Look direction (toward origin)
point r = {1, -1, 0};   // Right vector
point u = {0, 0, 1};    // Up vector

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

        glColor3f(0, 0, 1);
        // Bottom
        glVertex3f(a, -a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, -a, -a);
        glVertex3f(a, -a, -a);
    }
    glEnd();
}

void drawSS()
{
    glPushMatrix();
    glColor3f(0, 1, 0);
    // glTranslatef(0, 0, 0);
    // glRotatef(0, 1, 1, 1);
    drawCube(60);
    glPopMatrix();
}

void keyboardListener(unsigned char key, int x, int y)
{
    double rate = 0.05;
    point temp;

    switch (key)
    {
    case '1': // yaw right
        temp.x = l.x * cos(-rate) + r.x * sin(-rate);
        temp.y = l.y * cos(-rate) + r.y * sin(-rate);
        temp.z = l.z * cos(-rate) + r.z * sin(-rate);
        r.x = r.x * cos(-rate) - l.x * sin(-rate);
        r.y = r.y * cos(-rate) - l.y * sin(-rate);
        r.z = r.z * cos(-rate) - l.z * sin(-rate);
        l = temp;

        // Normalize vectors
        normalize(&l);
        normalize(&r);

        // Recalculate u to ensure orthogonality
        u.x = l.y * r.z - l.z * r.y;
        u.y = l.z * r.x - l.x * r.z;
        u.z = l.x * r.y - l.y * r.x;
        normalize(&u);
        break;

    case '2': // yaw left
        temp.x = l.x * cos(rate) + r.x * sin(rate);
        temp.y = l.y * cos(rate) + r.y * sin(rate);
        temp.z = l.z * cos(rate) + r.z * sin(rate);
        r.x = r.x * cos(rate) - l.x * sin(rate);
        r.y = r.y * cos(rate) - l.y * sin(rate);
        r.z = r.z * cos(rate) - l.z * sin(rate);
        l = temp;

        // Normalize vectors
        normalize(&l);
        normalize(&r);

        // Recalculate u to ensure orthogonality
        u.x = l.y * r.z - l.z * r.y;
        u.y = l.z * r.x - l.x * r.z;
        u.z = l.x * r.y - l.y * r.x;
        normalize(&u);
        break;

    case '3': // pitch up
        temp.x = l.x * cos(rate) + u.x * sin(rate);
        temp.y = l.y * cos(rate) + u.y * sin(rate);
        temp.z = l.z * cos(rate) + u.z * sin(rate);
        u.x = u.x * cos(rate) - l.x * sin(rate);
        u.y = u.y * cos(rate) - l.y * sin(rate);
        u.z = u.z * cos(rate) - l.z * sin(rate);
        l = temp;

        // Normalize vectors
        normalize(&l);
        normalize(&u);

        // Recalculate r to ensure orthogonality
        r.x = u.y * l.z - u.z * l.y;
        r.y = u.z * l.x - u.x * l.z;
        r.z = u.x * l.y - u.y * l.x;
        normalize(&r);
        break;

    case '4': // pitch down
        temp.x = l.x * cos(-rate) + u.x * sin(-rate);
        temp.y = l.y * cos(-rate) + u.y * sin(-rate);
        temp.z = l.z * cos(-rate) + u.z * sin(-rate);
        u.x = u.x * cos(-rate) - l.x * sin(-rate);
        u.y = u.y * cos(-rate) - l.y * sin(-rate);
        u.z = u.z * cos(-rate) - l.z * sin(-rate);
        l = temp;

        // Normalize vectors
        normalize(&l);
        normalize(&u);

        // Recalculate r to ensure orthogonality
        r.x = u.y * l.z - u.z * l.y;
        r.y = u.z * l.x - u.x * l.z;
        r.z = u.x * l.y - u.y * l.x;
        normalize(&r);
        break;

    case '5': // roll clockwise
        temp.x = r.x * cos(rate) + u.x * sin(rate);
        temp.y = r.y * cos(rate) + u.y * sin(rate);
        temp.z = r.z * cos(rate) + u.z * sin(rate);
        u.x = u.x * cos(rate) - r.x * sin(rate);
        u.y = u.y * cos(rate) - r.y * sin(rate);
        u.z = u.z * cos(rate) - r.z * sin(rate);
        r = temp;

        // Normalize vectors
        normalize(&r);
        normalize(&u);

        // Recalculate l to ensure orthogonality
        l.x = r.y * u.z - r.z * u.y;
        l.y = r.z * u.x - r.x * u.z;
        l.z = r.x * u.y - r.y * u.x;
        normalize(&l);
        break;

    case '6': // roll counter-clockwise
        temp.x = r.x * cos(-rate) + u.x * sin(-rate);
        temp.y = r.y * cos(-rate) + u.y * sin(-rate);
        temp.z = r.z * cos(-rate) + u.z * sin(-rate);
        u.x = u.x * cos(-rate) - r.x * sin(-rate);
        u.y = u.y * cos(-rate) - r.y * sin(-rate);
        u.z = u.z * cos(-rate) - r.z * sin(-rate);
        r = temp;

        // Normalize vectors
        normalize(&r);
        normalize(&u);

        // Recalculate l to ensure orthogonality
        l.x = r.y * u.z - r.z * u.y;
        l.y = r.z * u.x - r.x * u.z;
        l.z = r.x * u.y - r.y * u.x;
        normalize(&l);
        break;

    case 'w':
        pos.x += u.x * 2;
        pos.y += u.y * 2;
        pos.z += u.z * 2;
        break;

    case 's':
        pos.x -= u.x * 2;
        pos.y -= u.y * 2;
        pos.z -= u.z * 2;
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
        pos.x += l.x * 2;
        pos.y += l.y * 2;
        pos.z += l.z * 2;
        break;
    case GLUT_KEY_DOWN:
        pos.x -= l.x * 2;
        pos.y -= l.y * 2;
        pos.z -= l.z * 2;
        break;
    case GLUT_KEY_RIGHT:
        pos.x += r.x * 2;
        pos.y += r.y * 2;
        pos.z += r.z * 2;
        break;
    case GLUT_KEY_LEFT:
        pos.x -= r.x * 2;
        pos.y -= r.y * 2;
        pos.z -= r.z * 2;
        break;
    case GLUT_KEY_PAGE_UP:
        pos.x += u.x * 2;
        pos.y += u.y * 2;
        pos.z += u.z * 2;
        break;
    case GLUT_KEY_PAGE_DOWN:
        pos.x -= u.x * 2;
        pos.y -= u.y * 2;
        pos.z -= u.z * 2;
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

    gluLookAt(pos.x, pos.y, pos.z,
              pos.x + l.x, pos.y + l.y, pos.z + l.z,
              u.x, u.y, u.z);

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
    normalize(&l);
    normalize(&r);
    normalize(&u);

    // Make sure the vectors are orthogonal
    // First, make sure right is perpendicular to look
    // This is the cross product of world-up and look
    r.x = u.y * l.z - u.z * l.y;
    r.y = u.z * l.x - u.x * l.z;
    r.z = u.x * l.y - u.y * l.x;
    normalize(&r);

    // Then recompute up as cross product of look and right
    u.x = l.y * r.z - l.z * r.y;
    u.y = l.z * r.x - l.x * r.z;
    u.z = l.x * r.y - l.y * r.x;
    normalize(&u);

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