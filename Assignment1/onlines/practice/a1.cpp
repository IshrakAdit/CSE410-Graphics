#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define PI (2 * acos(0.0))

// Variables: Clock dimensions
const float CLOCK_RADIUS = 55.0f;
const float HAND_LENGTH = CLOCK_RADIUS;
const float HAND_WIDTH = 3.0f;

// Clock center position
float centerX = 0.0f;
float centerY = 0.0f;

// Sine wave parameters
const int MAX_WAVE_POINTS = 500;
float wavePoints[MAX_WAVE_POINTS][2]; // [x, y] coordinates
int wavePointCount = 0;
float currentAngle = 0.0f;

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

void drawHand(float angle)
{
    // Calculate hand position
    float radians = (90.0f - angle) * PI / 180.0f;
    float x = HAND_LENGTH * cosf(radians);
    float y = HAND_LENGTH * sinf(radians);

    // Draw the hand
    glLineWidth(HAND_WIDTH);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(centerX, centerY);
    glVertex2f(centerX + x, centerY + y);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(centerX + x, centerY + y);
    glVertex2f(centerX + CLOCK_RADIUS + 30, centerY + y);
    glEnd();

    // Store the tip position for sine wave
    float tipX = centerX + x;
    float tipY = centerY + y;

    // Update the wave points array
    if (wavePointCount < MAX_WAVE_POINTS)
    {
        wavePointCount++;
    }
    else
    {
        // Shift all points left
        for (int i = 0; i < MAX_WAVE_POINTS - 1; i++)
        {
            wavePoints[i][0] = wavePoints[i + 1][0];
            wavePoints[i][1] = wavePoints[i + 1][1];
        }
    }

    // Add new point at the end
    int lastIndex = wavePointCount - 1;

    // If this is the first point, use the tip position exactly
    if (lastIndex == 0)
    {
        wavePoints[lastIndex][0] = tipX;
        wavePoints[lastIndex][1] = tipY;
    }
    else
    {
        // Otherwise, add a small increment to X to create the wave moving right
        float prevX = wavePoints[lastIndex - 1][0];
        wavePoints[lastIndex][0] = prevX + 1.0f; // Increment X position for wave movement
        wavePoints[lastIndex][1] = tipY;
    }
}

void drawSineWave()
{
    if (wavePointCount < 2)
    {
        return; // Need at least 2 points to draw a line
    }

    glLineWidth(2.0f);
    glColor3f(0.0f, 1.0f, 1.0f); // Cyan color for the wave

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < wavePointCount; i++)
    {
        glVertex2f(wavePoints[i][0], wavePoints[i][1]);
    }
    glEnd();
}

void drawClock(float angle)
{
    // outer rim
    glColor3f(0.0f, .0f, 1.0f);
    drawCircle(centerX, centerY, CLOCK_RADIUS, 100);

    // Sine wave (draw first so the hand appears on top)
    drawSineWave();

    // Single hand
    drawHand(angle * 10);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Update counter-clockwise angle
    currentAngle -= 1.0f;
    if (currentAngle < 0)
    {
        currentAngle += 360.0f;
    }

    drawClock(currentAngle);

    glutSwapBuffers();
}

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(33, timer, 0); // ~30 FPS
}

void reshape(int w, int h)
{
    // Ensure aspect ratio is maintained for a round clock
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate aspect ratio
    float aspect = (float)w / (float)h;

    // Set the coordinate system to maintain the aspect ratio
    if (w <= h)
    {
        gluOrtho2D(-200.0, 400.0, -300.0 / aspect, 300.0 / aspect);
    }
    else
    {
        gluOrtho2D(-200.0 * aspect, 400.0 * aspect, -300.0, 300.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-200, 400, -300, 300); // Square aspect ratio for round clock
    glMatrixMode(GL_MODELVIEW);

    // Initialize wave points
    for (int i = 0; i < MAX_WAVE_POINTS; i++)
    {
        wavePoints[i][0] = 0.0f;
        wavePoints[i][1] = 0.0f;
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600); // More square aspect ratio
    glutCreateWindow("OpenGL - Clock with Sine Wave");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape); // Add reshape function to maintain aspect ratio
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}