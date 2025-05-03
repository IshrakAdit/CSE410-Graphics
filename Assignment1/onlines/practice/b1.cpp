#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define PI (2 * acos(0.0))

// Variables: Clock dimensions
const float CLOCK_RADIUS = 55.0f;
const float HOUR_HAND_LENGTH = 20.0f;
const float MINUTE_HAND_LENGTH = 30.0f;
const float SECOND_HAND_LENGTH = CLOCK_RADIUS;

const float HOUR_HAND_WIDTH = 4.0f;
const float MINUTE_HAND_WIDTH = 3.0f;
const float SECOND_HAND_WIDTH = 2.0f;

// Clock center position
float centerX = 0.0f;
float centerY = 0.0f;

// Draw Functions
void drawCircle(float cx, float cy, float radius, int segments)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
    {
        float theta = 2.0f * PI * (float)i / (float)segments;
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawThirdCircle(float second_angle)
{
    // outer rim
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCircle(centerX, centerY, CLOCK_RADIUS / 4, 100);

    // hands
    // drawSecondHand(second_angle * 5, SECOND_HAND_LENGTH / 2, SECOND_HAND_WIDTH, 0.0f, 0.0f, 1.0f);
}

void drawSecondHand(float angle, float length, float width, float r, float g, float b)
{
    // main hands
    float radians = (90.0f - angle) * PI / 180.0f;
    float x = length * cosf(radians);
    float y = length * sinf(radians);

    glLineWidth(width);
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY);
    glVertex2f(centerX + x, centerY + y);
    glEnd();

    // second circle
    x = (CLOCK_RADIUS / 2) * cosf(radians);
    y = (CLOCK_RADIUS / 2) * sinf(radians);

    glPushMatrix();
    glTranslatef(centerX + x, centerY + y, 0);
    drawThirdCircle(angle);
    glPopMatrix();
}

void drawSecondCircle(float second_angle)
{
    // outer rim
    glColor3f(0.0f, 0.0f, 1.0f);
    drawCircle(centerX, centerY, CLOCK_RADIUS / 2, 100);

    // hands
    drawSecondHand(second_angle * 5, SECOND_HAND_LENGTH / 2, SECOND_HAND_WIDTH, 0.0f, 0.0f, 1.0f);
}

void drawFirstHand(float angle, float length, float width, float r, float g, float b)
{
    // main hands
    float radians = (90.0f - angle) * PI / 180.0f;
    float x = length * cosf(radians);
    float y = length * sinf(radians);

    glLineWidth(width);
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY);
    glVertex2f(centerX + x, centerY + y);
    glEnd();

    // second circle
    x = CLOCK_RADIUS * cosf(radians);
    y = CLOCK_RADIUS * sinf(radians);

    glPushMatrix();
    glTranslatef(centerX + x, centerY + y, 0);
    drawSecondCircle(angle);
    glPopMatrix();
}

void drawFirstCircle(float second_angle)
{
    // outer rim
    glColor3f(1.0f, 0.0f, 0.0f);
    drawCircle(centerX, centerY, CLOCK_RADIUS, 100);

    // hands
    drawFirstHand(second_angle * 10, SECOND_HAND_LENGTH, SECOND_HAND_WIDTH, 1.0f, 0.0f, 0.0f);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;
    int second = timeinfo->tm_sec;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    float milliseconds = ts.tv_nsec / 1000000.0f;

    float hour_angle = (hour % 12) * 30.0f + (minute / 60.0f) * 30.0f;
    float minute_angle = minute * 6.0f;
    float second_angle = second * 6.0f + (milliseconds / 1000.0f) * 6.0f;

    drawFirstCircle(-second_angle);

    glutSwapBuffers();
}

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-100, 100, -100, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("OpenGL - Analog Clock");

    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}