#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define PI (2 * acos(0.0))

// Variables: Clock dimensions
const float CLOCK_RADIUS = 55.0f;
const float HOUR_HAND_LENGTH = 20.0f;
const float MINUTE_HAND_LENGTH = 30.0f;
const float SECOND_HAND_LENGTH = 40.0f;

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

void drawClockMarkers()
{
    // hour markers
    int HOUR_MARKER_START = 10;
    int HOUR_MARKER_END = 3;

    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int hour = 0; hour < 12; hour++)
    {
        float angle = hour * 30.0f * PI / 180.0f; // 30 degrees per hour
        float innerX = (CLOCK_RADIUS - HOUR_MARKER_START) * cosf(angle);
        float innerY = (CLOCK_RADIUS - HOUR_MARKER_START) * sinf(angle);
        float outerX = (CLOCK_RADIUS - HOUR_MARKER_END) * cosf(angle);
        float outerY = (CLOCK_RADIUS - HOUR_MARKER_END) * sinf(angle);

        glVertex2f(centerX + outerX, centerY + outerY);
        glVertex2f(centerX + innerX, centerY + innerY);
    }
    glEnd();

    // minute markers
    int MINUTE_MARKER_START = 6;
    int MINUTE_MARKER_END = 3;

    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int minute = 0; minute < 60; minute++)
    {
        // Skipping positions where hour markers are placed
        if (minute % 5 == 0)
            continue;

        float angle = minute * 6.0f * PI / 180.0f; // 6 degrees per minute
        float innerX = (CLOCK_RADIUS - MINUTE_MARKER_START) * cosf(angle);
        float innerY = (CLOCK_RADIUS - MINUTE_MARKER_START) * sinf(angle);
        float outerX = (CLOCK_RADIUS - MINUTE_MARKER_END) * cosf(angle);
        float outerY = (CLOCK_RADIUS - MINUTE_MARKER_END) * sinf(angle);

        glVertex2f(centerX + outerX, centerY + outerY);
        glVertex2f(centerX + innerX, centerY + innerY);
    }
    glEnd();
}

void drawSquare(double a, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    {
        glVertex3f(a, a, 0);
        glVertex3f(a, -a, 0);
        glVertex3f(-a, -a, 0);
        glVertex3f(-a, a, 0);
    }
    glEnd();
}

void drawHand(float angle, float length, float width, float r, float g, float b)
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

    // squares
    x = CLOCK_RADIUS * cosf(radians);
    y = CLOCK_RADIUS * sinf(radians);

    glPushMatrix();
    glTranslatef(centerX + x, centerY + y, 0);
    drawSquare(2, r, g, b);
    glPopMatrix();
}

void drawClock(float hour_angle, float minute_angle, float second_angle)
{
    // outer rim
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(centerX, centerY, CLOCK_RADIUS, 100);

    // markers
    drawClockMarkers();

    // hands
    drawHand(hour_angle, HOUR_HAND_LENGTH, HOUR_HAND_WIDTH, 1.0f, 1.0f, 1.0f);
    drawHand(minute_angle, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, 1.0f, 1.0f, 1.0f);
    drawHand(second_angle, SECOND_HAND_LENGTH, SECOND_HAND_WIDTH, 1.0f, 0.0f, 0.0f);
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

    drawClock(hour_angle, minute_angle, second_angle);

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