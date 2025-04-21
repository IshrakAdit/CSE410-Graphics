#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#define PI 3.14159265358979323846

// Clock dimensions
const float CLOCK_RADIUS = 80.0f;
const float HOUR_HAND_LENGTH = 50.0f;
const float MINUTE_HAND_LENGTH = 70.0f;
const float SECOND_HAND_LENGTH = 75.0f;

const float HOUR_HAND_WIDTH = 5.0f;
const float MINUTE_HAND_WIDTH = 3.0f;
const float SECOND_HAND_WIDTH = 1.5f;

// Clock center position
float centerX = 0.0f;
float centerY = 0.0f;

// Function to draw a circle using OpenGL lines
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

// Draw hour markers on the clock face
void drawClockMarkers()
{
    // Draw hour markers (thicker lines)
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int hour = 0; hour < 12; hour++)
    {
        float angle = hour * 30.0f * PI / 180.0f; // 30 degrees per hour
        float outerX = (CLOCK_RADIUS - 5) * cosf(angle);
        float outerY = (CLOCK_RADIUS - 5) * sinf(angle);
        float innerX = (CLOCK_RADIUS - 15) * cosf(angle);
        float innerY = (CLOCK_RADIUS - 15) * sinf(angle);

        glVertex2f(centerX + outerX, centerY + outerY);
        glVertex2f(centerX + innerX, centerY + innerY);
    }
    glEnd();

    // Draw minute markers (thinner lines)
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glColor3f(0.8f, 0.8f, 0.8f);

    for (int minute = 0; minute < 60; minute++)
    {
        // Skip positions where hour markers are placed
        if (minute % 5 == 0)
            continue;

        float angle = minute * 6.0f * PI / 180.0f; // 6 degrees per minute
        float outerX = (CLOCK_RADIUS - 5) * cosf(angle);
        float outerY = (CLOCK_RADIUS - 5) * sinf(angle);
        float innerX = (CLOCK_RADIUS - 10) * cosf(angle);
        float innerY = (CLOCK_RADIUS - 10) * sinf(angle);

        glVertex2f(centerX + outerX, centerY + outerY);
        glVertex2f(centerX + innerX, centerY + innerY);
    }
    glEnd();
}

// Draw the clock face and rim
void drawClockFace()
{
    // Draw outer rim
    glLineWidth(2.0f);
    glColor3f(0.7f, 0.7f, 0.7f);
    drawCircle(centerX, centerY, CLOCK_RADIUS, 100);

    // Draw inner face
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        float theta = 2.0f * PI * (float)i / 100.0f;
        float x = (CLOCK_RADIUS - 2) * cosf(theta);
        float y = (CLOCK_RADIUS - 2) * sinf(theta);
        glVertex2f(x + centerX, y + centerY);
    }
    glEnd();

    // Draw center circle
    glColor3f(0.8f, 0.8f, 0.8f);
    drawCircle(centerX, centerY, 3.0f, 20);

    // Draw hour and minute markers
    drawClockMarkers();
}

// Draw a clock hand with given angle, length, and width
void drawHand(float angle, float length, float width, float r, float g, float b)
{
    float radians = (90.0f - angle) * PI / 180.0f; // Adjust to start from 12 o'clock position
    float x = length * cosf(radians);
    float y = length * sinf(radians);

    glLineWidth(width);
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY);
    glVertex2f(centerX + x, centerY + y);
    glEnd();
}

// Display function
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Get current time
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;
    int second = timeinfo->tm_sec;

    // Convert to angles
    float hourAngle = (hour % 12) * 30.0f + (minute / 60.0f) * 30.0f; // 30 degrees per hour
    float minuteAngle = minute * 6.0f + (second / 60.0f) * 6.0f;      // 6 degrees per minute
    float secondAngle = second * 6.0f;                                // 6 degrees per second

    // Draw the clock face
    drawClockFace();

    // Draw the hands
    drawHand(hourAngle, HOUR_HAND_LENGTH, HOUR_HAND_WIDTH, 1.0f, 0.5f, 0.5f);       // Hour hand (reddish)
    drawHand(minuteAngle, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, 0.5f, 1.0f, 0.5f); // Minute hand (greenish)
    drawHand(secondAngle, SECOND_HAND_LENGTH, SECOND_HAND_WIDTH, 0.5f, 0.5f, 1.0f); // Second hand (bluish)

    glutSwapBuffers();
}

// Reshape function to maintain aspect ratio
void reshape(int width, int height)
{
    float aspectRatio = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (width >= height)
    {
        gluOrtho2D(-100 * aspectRatio, 100 * aspectRatio, -100, 100);
    }
    else
    {
        gluOrtho2D(-100, 100, -100 / aspectRatio, 100 / aspectRatio);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Timer function for animation
void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS refresh rate
}

// Initialize OpenGL settings
void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
    glutCreateWindow("OpenGL Analog Clock");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}