#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <cmath>

struct vector_3d
{
    GLdouble x, y, z;
};

struct point
{
    GLdouble x, y, z;
};

char title[] = "Wind Mill"; // Windowed mode's title
int windowWidth = 600;      // Windowed mode's width
int windowHeight = 600;     // Windowed mode's height
int windowPosX = 50;        // Windowed mode's top-left corner x
int windowPosY = 50;        // Windowed mode's top-left corner y

unsigned int refreshMillis = 10;  // Refresh interval in milliseconds
GLfloat perspectiveAngle = 45.0f; // Perspective angle

point eye = {1 / sqrt(3), 1 / sqrt(3), 1 / sqrt(3)};                  // Camera position
vector_3d forwardVector = {-1 / sqrt(3), -1 / sqrt(3), -1 / sqrt(3)}; // Forward vector
vector_3d rightVector = {1 / sqrt(2), 0.0, -1 / sqrt(2)};             // Right vector
vector_3d upVector = {-1 / sqrt(6), 2 / sqrt(6), -1 / sqrt(6)};       // Up vector

GLfloat baseRotateAngele = 0.0f;
GLfloat rotationAngle = 0.0f;
GLfloat rotationSpeed = 1.0;

GLfloat baseBoundary = 0.05;
GLfloat poleBoundary = 0.01;

void initGLUT(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(windowPosX, windowPosY);
    glutCreateWindow(title);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
    glEnable(GL_DEPTH_TEST);              // Enable depth testing for z-culling
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // Reset projection matrix
    gluPerspective(perspectiveAngle, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.01f, 10.0f);
    glMatrixMode(GL_MODELVIEW);
}

vector_3d crossProduct(vector_3d A, vector_3d B)
{
    vector_3d C;
    C.x = A.y * B.z - A.z * B.y;
    C.y = A.z * B.x - A.x * B.z;
    C.z = A.x * B.y - A.y * B.x;
    return C;
}

// Function to compute the scalar product of two vectors A and scalar t
vector_3d scalarProduct(vector_3d A, GLdouble t)
{
    vector_3d C;
    C.x = A.x * t;
    C.y = A.y * t;
    C.z = A.z * t;
    return C;
}

// Function to compute the sum of two vectors A and B
vector_3d sum(vector_3d A, vector_3d B, GLdouble t, GLdouble s)
{
    vector_3d C;
    C.x = A.x * t + B.x * s;
    C.y = A.y * t + B.y * s;
    C.z = A.z * t + B.z * s;
    return C;
}

// Function to Compute the sum of a point and a vector
point sum(point A, vector_3d B, GLdouble t)
{
    point C;
    C.x = A.x + t * B.x;
    C.y = A.y + t * B.y;
    C.z = A.z + t * B.z;
    return C;
}

// Function to determine vertor from two points
vector_3d vectorFromPoints(point A, point B)
{
    vector_3d C;
    C.x = B.x - A.x;
    C.y = B.y - A.y;
    C.z = B.z - A.z;
    return C;
}

// Function to normalize a vector A
void normalize(vector_3d &A)
{
    GLdouble length = sqrt(A.x * A.x + A.y * A.y + A.z * A.z);
    A.x /= length;
    A.y /= length;
    A.z /= length;
}

// Function to compute the projection of a vector onto a plane
vector_3d projectVectorOntoPlane(vector_3d v, vector_3d planeNormal)
{
    // Normalize the plane normal
    normalize(planeNormal);

    // Compute the dot product of v and the plane normal
    GLdouble dotProduct = v.x * planeNormal.x + v.y * planeNormal.y + v.z * planeNormal.z;

    // Compute the projection
    vector_3d projection = sum(v, planeNormal, 1.0, -dotProduct);

    return projection;
}

// Function to handle keyboard input
void specialKeyListener(int key, int x, int y)
{
    GLfloat v = 0.01f; // Step size for movement
    switch (key)
    {
    case GLUT_KEY_UP: // Move camera forward
        eye = sum(eye, forwardVector, v);
        break;

    case GLUT_KEY_DOWN: // Move camera backward
        eye = sum(eye, forwardVector, -v);
        break;

    case GLUT_KEY_LEFT: // Move camera left
        eye = sum(eye, rightVector, -v);
        break;

    case GLUT_KEY_RIGHT: // Move camera right
        eye = sum(eye, rightVector, v);
        break;

    case GLUT_KEY_PAGE_UP: // Move camera upward
        eye = sum(eye, upVector, v);
        break;

    case GLUT_KEY_PAGE_DOWN: // Move camera downward
        eye = sum(eye, upVector, -v);
        break;
    }
}

// Function to handle keyboard input
void keyboardListener(unsigned char key, int x, int y)
{
    GLfloat v = 100.0f; // Step size for movement
    switch (key)
    {
    case '1': // Rotate camera right (yaw)
        forwardVector = sum(forwardVector, rightVector, v, -1);
        normalize(forwardVector);
        rightVector = crossProduct(forwardVector, upVector);
        break;
    case '2': // Rotate camera left (yaw)
        forwardVector = sum(forwardVector, rightVector, v, 1);
        normalize(forwardVector);
        rightVector = crossProduct(forwardVector, upVector);
        break;
    case '3': // Rotate camera up (pitch)
        forwardVector = sum(forwardVector, upVector, v, 1);
        normalize(forwardVector);
        upVector = crossProduct(rightVector, forwardVector);
        break;
    case '4': // Rotate camera down (pitch)
        forwardVector = sum(forwardVector, upVector, v, -1);
        normalize(forwardVector);
        upVector = crossProduct(rightVector, forwardVector);
        break;
    case '5': // Rotate camera counterclockwise (roll)
        rightVector = sum(rightVector, upVector, v, -1);
        normalize(rightVector);
        upVector = crossProduct(rightVector, forwardVector);
        break;
    case '6': // Rotate camera clockwise (roll)
        rightVector = sum(rightVector, upVector, v, 1);
        normalize(rightVector);
        upVector = crossProduct(rightVector, forwardVector);
        break;
    case 'w':
    {
        rotationSpeed += 1;

        break;
    }
    case 's':
    {
        if (rotationSpeed > 1)
            rotationSpeed -= 1;
        break;
    }
    case 'a':
        baseRotateAngele += 5.0f;
        if (baseRotateAngele > 360.0)
            baseRotateAngele -= 360.f;
        break;
    case 'd':
        baseRotateAngele -= 5.0f;
        if (baseRotateAngele < 360.0)
            baseRotateAngele += 360.f;
        break;
    }
    glutPostRedisplay(); // Request a screen refresh
}

point base[8] = {
    {baseBoundary, 5 * baseBoundary, baseBoundary},
    {-baseBoundary, 5 * baseBoundary, baseBoundary},
    {-baseBoundary, -5 * baseBoundary, baseBoundary},
    {baseBoundary, -5 * baseBoundary, baseBoundary},
    {baseBoundary, 5 * baseBoundary, -baseBoundary},
    {-baseBoundary, 5 * baseBoundary, -baseBoundary},
    {-baseBoundary, -5 * baseBoundary, -baseBoundary},
    {baseBoundary, -5 * baseBoundary, -baseBoundary}};

point pole[8] = {
    {10 * poleBoundary, poleBoundary, poleBoundary},
    {-10 * poleBoundary, poleBoundary, poleBoundary},
    {-10 * poleBoundary, -poleBoundary, poleBoundary},
    {10 * poleBoundary, -poleBoundary, poleBoundary},
    {10 * poleBoundary, poleBoundary, -poleBoundary},
    {-10 * poleBoundary, poleBoundary, -poleBoundary},
    {-10 * poleBoundary, -poleBoundary, -poleBoundary},
    {10 * poleBoundary, -poleBoundary, -poleBoundary}};

void drawLine(point p1, point p2)
{
    glBegin(GL_LINES);
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
}

// Function to draw the axes
void drawAxis()
{
    glBegin(GL_LINES);
    drawLine({-0.5f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}); // X-axis
    drawLine({0.0f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}); // Y-axis
    drawLine({0.0f, 0.0f, -0.5f}, {0.0f, 0.0f, 0.5f}); // Z-axis
    glEnd();
}

// Function to draw a cube using the specified points
void drawFace(point p1, point p2, point p3, point p4)
{
    glBegin(GL_QUADS);
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glVertex3f(p3.x, p3.y, p3.z);
    glVertex3f(p4.x, p4.y, p4.z);
    glEnd();
}

// The base will be 4 rectangular faces.
void drawBase(point p1, point p2, point p3, point p4,
              point p5, point p6, point p7, point p8)
{
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    drawFace(p1, p2, p3, p4);
    drawFace(p5, p6, p7, p8);

    glColor3f(0.0f, 1.0f, 0.0f); // Green
    drawFace(p1, p4, p8, p5);
    drawFace(p2, p3, p7, p6);
}

// a pole (drawn using 4 rectangular faces) securing the blades.
void drawPole(point p1, point p2, point p3, point p4,
              point p5, point p6, point p7, point p8)
{
    glColor3f(1.0f, 0.0f, 1.0f); // Magenta
    drawFace(p1, p2, p3, p4);
    drawFace(p5, p6, p7, p8);
    drawFace(p1, p5, p6, p2);
    drawFace(p3, p4, p8, p7);
}

void drawBlade(GLfloat radius)
{
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, radius, 0.05);
    glVertex3f(0.0, radius, -0.05);
    glEnd();
}

// 3 triangular blades at 120° apart
void drawWindMill()
{
    glPushMatrix();
    drawBlade(0.2);
    glPopMatrix();

    glPushMatrix();
    glRotatef(120, 1.0, 0.0, 0.0);
    drawBlade(0.2);
    glPopMatrix();

    glPushMatrix();
    glRotatef(240, 1.0, 0.0, 0.0);
    drawBlade(0.2);
    glPopMatrix();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Reset model-view matrix

    point center = sum(eye, forwardVector, 1.0f); // Calculate the center point to look at

    // Set the camera position
    gluLookAt(eye.x, eye.y, eye.z,                 // Camera position
              center.x, center.y, center.z,        // Look-at point
              upVector.x, upVector.y, upVector.z); // Up vector

    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white

    drawAxis(); // Draw the axes

    glRotatef(baseRotateAngele, 0.0f, 1.0f, 0.0f);

    // Draw the Base
    drawBase(base[0], base[1], base[2], base[3],
             base[4], base[5], base[6], base[7]);

    glTranslatef(0.0, 0.2, 0.0);

    drawPole(pole[0], pole[1], pole[2], pole[3],
             pole[4], pole[5], pole[6], pole[7]);

    glTranslatef(baseBoundary + 0.05, 0.0, 0.0);

    glRotatef(rotationAngle, 1.0, 0.0, 0.0);

    glColor3f(0.0f, 1.0f, 1.0f); // Cyan
    drawWindMill();

    glFlush();         // Render now
    glutSwapBuffers(); // Swap front and back buffers (of double buffered mode)
}

void timerFunction(int value)
{
    rotationAngle += rotationSpeed;
    // Request a redisplay
    glutPostRedisplay();

    // Register the timer again
    glutTimerFunc(refreshMillis, timerFunction, 0);
}

void reshape(GLsizei width, GLsizei height)
{
    // Prevent division by zero
    if (height == 0)
        height = 1;

    // Calculate aspect ratio
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set viewport to cover entire window
    glViewport(0, 0, width, height);

    // Set up perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 45-degree field of view, aspect ratio, near and far clipping planes
    gluPerspective(perspectiveAngle, aspect, 0.01f, 10.0f);

    // Switch back to the model-view matrix
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    initGLUT(&argc, argv); // Initialize GLUT

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutTimerFunc(refreshMillis, timerFunction, 0);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}