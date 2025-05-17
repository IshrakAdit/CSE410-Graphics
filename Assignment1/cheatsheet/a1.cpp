#include <bits/stdc++.h>
#include <chrono>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

using namespace std;

#define PI 3.14159265f

void axes();
void circleOutline(double x, double y, double r);
void drawLine(double x1, double y1, double x2, double y2);
void square(double a);
void squareOutline(double a);
void drawFilledCircle(float x, float y, float radius, int segments);
void drawTriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3);
void drawTriangleOutline(float x1, float y1, float x2, float y2, float x3, float y3);

float line = 0;
float currentX = 0;
vector<float> points;
float wave_starting_point = 0.25;
int number_of_wave_points = 6000;
float circle_radius = 0.20;

void display()
{
    // glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT);

    // axes();
    glColor3f(1, 1, 1);
    circleOutline(0, 0, circle_radius);
    glPushMatrix();
    glRotatef(line, 0, 0, 1);
    drawLine(0, 0, circle_radius, 0);
    glPopMatrix();
    double rad = PI * line / 180;
    double x = circle_radius * cosf(rad);
    double y = circle_radius * sinf(rad);
    drawLine(x, y, wave_starting_point, y);
    // drawFilledCircle(x, y, 0.001, 1);
    // glTranslatef(-0.001, 0, 0);
    points.push_back(y);
    if (points.size() > number_of_wave_points)
    {
        points.erase(points.begin()); // keep it sliding
    }
    int n = points.size();
    float offset = 0;
    // printf("%d\n", n);
    glBegin(GL_LINE_STRIP);
    for (int i = n - 1; i >= 0; i--)
    {

        glVertex2d(wave_starting_point + offset, points[i]);
        offset += 0.0001;
    }
    glEnd();
    // glFlush();
    glutSwapBuffers();
}

void idle()
{

    line += 0.05;
    // Redraw the display
    glutPostRedisplay();
}

void init()
{
    glClearColor(0.0f, .0f, 0.0f, 1.0f); // Set background color to black and opaque
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowPosition(400, 400);
    glutInitWindowSize(800, 800);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutCreateWindow("OpenGL Demo");

    init();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();

    return 0;
}

void axes()
{
    glColor3f(0, 0, 1.0);
    glBegin(GL_LINES);
    {
        glVertex3f(100, 0, 0);
        glVertex3f(-100, 0, 0);

        glVertex3f(0, -100, 0);
        glVertex3f(0, 100, 0);
    }
    glEnd();
}

void drawFilledCircle(float x, float y, float radius, int segments)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center of the circle
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * M_PI * i / segments;
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

void circleOutline(double x, double y, double r)
{

    glPushMatrix();
    int segments = 100;
    glBegin(GL_LINE_LOOP); // Use line loop for border only
    for (int i = 0; i < segments; i++)
    {
        double theta = 2.0f * PI * double(i) / double(segments);
        double x = r * cosf(theta);
        double y = r * sinf(theta);
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();
}

void drawLine(double x1, double y1, double x2, double y2)
{
    glBegin(GL_LINES);
    {
        glVertex3f(x1, y1, 0);
        glVertex3f(x2, y2, 0);
    }
    glEnd();
}

void square(double a)
{

    glBegin(GL_QUADS);
    {
        glVertex3f(a, a, 0);
        glVertex3f(a, -a, 0);
        glVertex3f(-a, -a, 0);
        glVertex3f(-a, a, 0);
    }
    glEnd();
}

void squareOutline(double a)
{

    glBegin(GL_LINE_LOOP);
    {
        glVertex3f(a, a, 0);
        glVertex3f(a, -a, 0);
        glVertex3f(-a, -a, 0);
        glVertex3f(-a, a, 0);
    }
    glEnd();
}

void drawTriangleFilled(float x1, float y1, float x2, float y2, float x3, float y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

void drawTriangleOutline(float x1, float y1, float x2, float y2, float x3, float y3)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}
