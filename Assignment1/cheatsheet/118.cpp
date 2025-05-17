#include <GL/glut.h>

GLfloat eyex = 10, eyey = 10, eyez = 10;
GLfloat centerx = 0, centery = 0, centerz = 0;
GLfloat upx = 0, upy = 1, upz = 0;
float rotang = 0, bladespeed = 4, bladeang = 0;

#define PI 3.14159265358979323846

void initGL()
{
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 1.0, 100.0);
}

void reshapeListener(int w, int h)
{
    if (h == 0)
        h = 1;
    float aspect = (float)w / h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, aspect, 1.0, 100.0);
}

void drawCube()
{
    float width = 1.5f, height = 6.0f, depth = 1.5f; // Cube dimensions
    glBegin(GL_QUADS);
    glColor3f(0.9f, 0.5f, 0.2f);
    glVertex3f(width, height, depth);
    glVertex3f(-width, height, depth);
    glVertex3f(-width, -height, depth);
    glVertex3f(width, -height, depth);

    // Back face (z = -depth)
    glColor3f(0.867f, 0.922f, 0.122f);
    glVertex3f(width, -height, -depth);
    glVertex3f(-width, -height, -depth);
    glVertex3f(-width, height, -depth);
    glVertex3f(width, height, -depth);

    // Left face (x = -width)
    glColor3f(0.941f, 0.322f, 0.322f);
    glVertex3f(-width, height, depth);
    glVertex3f(-width, height, -depth);
    glVertex3f(-width, -height, -depth);
    glVertex3f(-width, -height, depth);

    // Right face (x = +width)
    glColor3f(0.408f, 0.859f, 0.298f);
    glVertex3f(width, height, -depth);
    glVertex3f(width, height, depth);
    glVertex3f(width, -height, depth);
    glVertex3f(width, -height, -depth);

    glEnd();
}
void drawAxes()
{
    glLineWidth(3); // Set line thickness

    glBegin(GL_LINES);

    // X axis (red)
    glColor3f(1, 1, 1);
    glVertex3f(-100, 0, 0);
    glVertex3f(100, 0, 0);

    // Y axis (green)
    glColor3f(1, 1, 1);
    glVertex3f(0, -100, 0);
    glVertex3f(0, 100, 0);

    // Z axis (blue)
    glColor3f(1, 1, 1);
    glVertex3f(0, 0, -100);
    glVertex3f(0, 0, 100);

    glEnd();
}
void drawCylinder(float baseRadius, float topRadius, float height)
{
    GLUquadric *quad = gluNewQuadric();
    glColor3f(0.7f, 0.7f, 0.7f); // Gray hub
    gluCylinder(quad, baseRadius, topRadius, height, 20, 20);
    gluDeleteQuadric(quad);
}
void drawBlade()
{
    glBegin(GL_TRIANGLES);
    glColor3f(0.53f, 0.81f, 0.92f);
    glVertex3f(0, 0, 2.05);
    glVertex3f(0, 2, 2.05);
    glVertex3f(1, 2, 2.05);
    glEnd();
}
void drawRotatingBlade()
{
    for (int i = 0; i < 3; i++)
    {
        glPushMatrix();
        glRotatef(i * 120, 0, 0, 1);
        drawBlade();
        glPopMatrix();
    }
}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
    drawAxes();
    glPushMatrix();
    glRotatef(rotang, 0, 1, 0); // Yaw for better visibility
    glColor3f(0.2f, 0.7f, 0.9f);
    drawCube();
    glTranslatef(0, 5, 0);
    glPushMatrix();
    drawCylinder(0.2f, 0.2f, 2.05f);
    glRotatef(bladeang, 0, 0, 1);
    drawRotatingBlade();
    glPopMatrix();
    glPopMatrix();

    glutSwapBuffers();
}

void keyboardListener(unsigned char key, int x, int y)
{
    if (key == 'a')
        rotang -= 2;
    if (key == 'd')
        rotang += 2;
    if (key == 'w')
        bladespeed += 3;
    if (key == 's')
        bladespeed -= 3;
    glutPostRedisplay();
}
void timerFunc(int value)
{
    bladeang += bladespeed;
    glutPostRedisplay();
    glutTimerFunc(16, timerFunc, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(640, 640);
    glutCreateWindow("OpenGL Debug Cube");

    initGL();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardListener);
    glutReshapeFunc(reshapeListener);
    glutTimerFunc(0, timerFunc, 0);
    glutMainLoop();
    return 0;
}
