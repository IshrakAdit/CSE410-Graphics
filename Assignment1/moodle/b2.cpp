#include <GL/glut.h>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>

using namespace std;
#define PI 3.14159265

float camX = 4, camY = 4, camZ = 4; // Camera position
float refX = 0, refY = 0, refZ = 0; // Reference point
float upX = 0, upY = 1, upZ = 0;    // Up vector

float stringLength = 0.5f;
float bottomRadius = 0.5f;
float seatOffset = 0.1f;
bool rotation = false;
bool axis = true;
int animationSpeed = 30;

float angle = 0;
float initangle = 0;

float rotationMatrix[16];

void cross(float vx, float vy, float vz, float ax, float ay, float az, float &resX, float &resY, float &resZ)
{
    resX = vy * az - vz * ay;
    resY = vz * ax - vx * az;
    resZ = vx * ay - vy * ax;
}

void normalize(float &vx, float &vy, float &vz)
{
    float length = sqrt(vx * vx + vy * vy + vz * vz);
    vx /= length;
    vy /= length;
    vz /= length;
}

void init()
{
    for (int i = 0; i < 16; i++)
        rotationMatrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

void draw_axes()
{
    glLineWidth(1);
    glBegin(GL_LINES);

    glColor3f(255, 255, 255);
    glVertex3f(2, 0, 0);
    glVertex3f(-2, 0, 0);

    glColor3f(255, 255, 255);
    glVertex3f(0, 2, 0);
    glVertex3f(0, -2, 0);

    glColor3f(255, 255, 255);
    glVertex3f(0, 0, 2);
    glVertex3f(0, 0, -2);

    glEnd();
}

void draw_circle()
{
    glColor3f(255, 255, 255);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i)
    {
        float theta = i * PI / 180.0;
        glVertex3f(0.5f * cos(theta), 0, 0.5f * sin(theta));
    }
    glEnd();
}

void draw_swing()
{

    glPushMatrix();
    if (rotation == true)
        glRotatef(angle, 0, 1, 0);
    glMultMatrixf(rotationMatrix);

    for (int i = 0; i < 360; i += 60)
    {
        float theta = i * PI / 180.0;

        float topX = 0.5f * cos(theta);
        float topZ = 0.5f * sin(theta);

        float bottomX1 = (bottomRadius - seatOffset / 2) * cos(theta);
        float bottomZ1 = (bottomRadius - seatOffset / 2) * sin(theta);

        float bottomX2 = (bottomRadius + seatOffset / 2) * cos(theta);
        float bottomZ2 = (bottomRadius + seatOffset / 2) * sin(theta);

        float dx1 = topX - bottomX1;
        float dz1 = topZ - bottomZ1;
        float dy1 = -sqrt(stringLength * stringLength - dx1 * dx1 - dz1 * dz1);

        float dx2 = topX - bottomX2;
        float dz2 = topZ - bottomZ2;
        float dy2 = -sqrt(stringLength * stringLength - dx2 * dx2 - dz2 * dz2);

        glLineWidth(1);
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f(topX, 0, topZ);
        glVertex3f(bottomX1, dy1, bottomZ1);
        glVertex3f(topX, 0, topZ);
        glVertex3f(bottomX2, dy2, bottomZ2);
        glEnd();

        float dx = bottomX2 - bottomX1;
        float dz = bottomZ2 - bottomZ1;
        float px = -dz, pz = dx;
        float len = sqrt(px * px + pz * pz);
        px /= len;
        pz /= len;
        float halfSize = 0.05f;

        float sx1 = bottomX1 + px * halfSize;
        float sz1 = bottomZ1 + pz * halfSize;

        float sx2 = bottomX2 + px * halfSize;
        float sz2 = bottomZ2 + pz * halfSize;

        float sx3 = bottomX2 - px * halfSize;
        float sz3 = bottomZ2 - pz * halfSize;

        float sx4 = bottomX1 - px * halfSize;
        float sz4 = bottomZ1 - pz * halfSize;

        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
        glVertex3f(sx1, dy1, sz1);
        glVertex3f(sx2, dy2, sz2);
        glVertex3f(sx3, dy2, sz3);
        glVertex3f(sx4, dy1, sz4);
        glEnd();
    }
    glPopMatrix();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, refX, refY, refZ, upX, upY, upZ);
    draw_circle();
    draw_swing();
    if (axis == true)
        draw_axes();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)w / h, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int)
{
    switch (key)
    {
    case '1':
    {
        rotation = true;
        if (bottomRadius < 0.9f)
        {
            initangle += 10;
            bottomRadius += 0.05f;
        }
        break;
    }
    case '2':
    {
        bottomRadius -= 0.05f;
        initangle -= 10;
        if (bottomRadius <= 0.5f)
        {
            rotation = false;
            bottomRadius = 0.5f;
            initangle = 0;
        }
        break;
    }
    }
    glutPostRedisplay();
}
void specialKeys(int key, int, int)
{
    float step = 0.5f;
    float dx = refX - camX;
    float dy = refY - camY;
    float dz = refZ - camZ;

    normalize(dx, dy, dz);

    float rx, ry, rz;
    cross(dx, dy, dz, upX, upY, upZ, rx, ry, rz);
    normalize(rx, ry, rz);

    switch (key)
    {
    case GLUT_KEY_UP:
        camY += step;
        break;

    case GLUT_KEY_DOWN:
        camY -= step;
        break;

    case GLUT_KEY_LEFT:
    {
        camX -= step * rx;
        camZ -= step * rz;
        float s2 = sqrt(camX * camX + camZ * camZ) / (4 * sqrt(2));
        camX /= s2;
        camZ /= s2;
        break;
    }
    case GLUT_KEY_RIGHT:
    {
        camX += step * rx;
        camZ += step * rz;
        float s1 = sqrt(camX * camX + camZ * camZ) / (4 * sqrt(2));
        camX /= s1;
        camZ /= s1;
        break;
    }
    }

    glutPostRedisplay();
}

void mouseListener(int button, int state, int x, int y)
{

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        axis = !axis;
        glutPostRedisplay();
    }
}

void timer(int value)
{
    if (rotation == true)
        angle += initangle * 0.2;
    glutPostRedisplay();
    glutTimerFunc(animationSpeed, timer, 0);
}

int main(int argc, char **argv)
{
    srand(time(0));
    init();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("swing");
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseListener);
    glutTimerFunc(animationSpeed, timer, 0);
    glutMainLoop();
    return 0;
}
