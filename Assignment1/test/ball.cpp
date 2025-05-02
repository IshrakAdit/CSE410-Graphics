#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define pi (2 * acos(0.0))
#define GRAVITY 9.8
#define RESTITUTION 0.75
#define MIN_VELOCITY 0.1
#define CUBE_SIZE 120.0
#define BALL_RADIUS 5.0 // Smaller ball radius
#define ARROW_SCALE 0.5

struct point
{
    double x, y, z;
};

// Initial camera position and direction vectors
point pos = {100, 100, 100};
point l = {-1, -1, -1}; // Look direction (toward origin)
point r = {1, -1, 0};   // Right vector
point u = {0, 0, 1};    // Up vector

// Ball properties
point ballPos = {0, 0, BALL_RADIUS};
point ballVel = {0, 0, 0};
point initialVel = {0, 0, 40};
double ballRotationAngle = 0;
point ballRotationAxis = {1, 0, 0};

// Simulation controls
int drawgrid = 0;
int drawaxes = 0;
int simRunning = 0;
int showVelocityArrow = 1;
double initialSpeed = 40.0;

// Utility function to normalize a vector
void normalize(point *p)
{
    double len = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
    if (len < 0.0001)
    {
        p->x = p->y = 0;
        p->z = 1;
        return;
    }
    p->x /= len;
    p->y /= len;
    p->z /= len;
}

// Get the magnitude of a vector
double magnitude(point p)
{
    return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

// Cross product of two vectors
point crossProduct(point a, point b)
{
    point result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// Reset ball with random position and velocity
void resetBall()
{
    double halfCube = CUBE_SIZE / 2;

    // Random position within the cube
    ballPos.x = (double)(rand() % 80 - 40);
    ballPos.y = (double)(rand() % 80 - 40);
    ballPos.z = BALL_RADIUS + (double)(rand() % 20);

    // Random initial velocity in any direction (not just upward)
    double theta = (double)(rand() % 360) * pi / 180.0; // Horizontal angle
    double phi = (double)(rand() % 180) * pi / 180.0;   // Vertical angle

    ballVel.x = initialSpeed * sin(phi) * cos(theta);
    ballVel.y = initialSpeed * sin(phi) * sin(theta);
    ballVel.z = initialSpeed * cos(phi);

    // Reset rotation
    ballRotationAngle = 0;
    ballRotationAxis.x = 1;
    ballRotationAxis.y = 0;
    ballRotationAxis.z = 0;

    // Stop simulation
    simRunning = 0;
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

// Draw a checkered floor within the cube bounds
void drawFloor()
{
    int i, j;
    int tileSize = 15;
    double halfCube = CUBE_SIZE / 2;
    int numTiles = (int)(CUBE_SIZE / tileSize);

    glBegin(GL_QUADS);
    for (i = -numTiles / 2; i < numTiles / 2; i++)
    {
        for (j = -numTiles / 2; j < numTiles / 2; j++)
        {
            // Make sure tiles stay within cube bounds
            if (i * tileSize + tileSize > halfCube || i * tileSize < -halfCube ||
                j * tileSize + tileSize > halfCube || j * tileSize < -halfCube)
            {
                continue;
            }

            if ((i + j) % 2 == 0)
            {
                glColor3f(1.0, 1.0, 1.0); // White
            }
            else
            {
                glColor3f(0.0, 0.0, 0.0); // Black
            }

            glVertex3f(i * tileSize, j * tileSize, 0);
            glVertex3f(i * tileSize, (j + 1) * tileSize, 0);
            glVertex3f((i + 1) * tileSize, (j + 1) * tileSize, 0);
            glVertex3f((i + 1) * tileSize, j * tileSize, 0);
        }
    }
    glEnd();
}

// Draw a cube with different colored sides
void drawCube()
{
    double a = CUBE_SIZE / 2;

    glBegin(GL_QUADS);
    {
        // Bottom - no need to draw, floor replaces it

        // Top (ceiling)
        glColor3f(0.3, 0.3, 0.8); // Blue ceiling
        glVertex3f(a, a, a);
        glVertex3f(a, -a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, a, a);

        // Sides
        glColor3f(0.8, 0.3, 0.3); // Red
        glVertex3f(a, a, 0);
        glVertex3f(a, a, a);
        glVertex3f(-a, a, a);
        glVertex3f(-a, a, 0);

        glColor3f(0.3, 0.8, 0.3); // Green
        glVertex3f(a, -a, 0);
        glVertex3f(a, -a, a);
        glVertex3f(a, a, a);
        glVertex3f(a, a, 0);

        glColor3f(0.8, 0.5, 0.2); // Orange
        glVertex3f(-a, -a, 0);
        glVertex3f(-a, -a, a);
        glVertex3f(a, -a, a);
        glVertex3f(a, -a, 0);

        glColor3f(0.5, 0.2, 0.8); // Purple
        glVertex3f(-a, a, 0);
        glVertex3f(-a, a, a);
        glVertex3f(-a, -a, a);
        glVertex3f(-a, -a, 0);
    }
    glEnd();
}

void drawBall(double radius, int slices, int stacks)
{
    glPushMatrix();
    glRotatef(ballRotationAngle * 180.0 / pi,
              ballRotationAxis.x,
              ballRotationAxis.y,
              ballRotationAxis.z);

    // Loop over vertical slices (longitude)
    for (int j = 0; j < slices; j++)
    {
        double lng0 = 2 * pi * (double)(j) / slices;
        double lng1 = 2 * pi * (double)(j + 1) / slices;

        // Split into upper and lower hemispheres
        // ----- UPPER HALF -----
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= stacks / 2; i++) // From equator up to top
        {
            double lat = pi * (-0.5 + (double)i / stacks);
            double z = sin(lat);
            double r = cos(lat);

            double x0 = cos(lng0);
            double y0 = sin(lng0);
            double x1 = cos(lng1);
            double y1 = sin(lng1);

            // Coloring upper half
            if (j % 2 == 0)
                glColor3f(0.9, 0.1, 0.1); // Red
            else
                glColor3f(0.1, 0.9, 0.1); // Green

            glVertex3f(x0 * r * radius, y0 * r * radius, z * radius);
            glVertex3f(x1 * r * radius, y1 * r * radius, z * radius);
        }
        glEnd();

        // ----- LOWER HALF -----
        glBegin(GL_QUAD_STRIP);
        for (int i = stacks / 2; i <= stacks; i++) // From equator down to bottom
        {
            double lat = pi * (-0.5 + (double)i / stacks);
            double z = sin(lat);
            double r = cos(lat);

            double x0 = cos(lng0);
            double y0 = sin(lng0);
            double x1 = cos(lng1);
            double y1 = sin(lng1);

            // Coloring lower half
            if (j % 2 == 0)
                glColor3f(0.1, 0.9, 0.1); // Green
            else
                glColor3f(0.9, 0.1, 0.1); // Red

            glVertex3f(x0 * r * radius, y0 * r * radius, z * radius);
            glVertex3f(x1 * r * radius, y1 * r * radius, z * radius);
        }
        glEnd();
    }

    glPopMatrix();
}

// Draw a sphere representing the ball with red and green stripes
void drawSphere(double radius, int slices, int stacks)
{
    GLUquadric *quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);

    // Draw a textured sphere
    glPushMatrix();
    glRotatef(ballRotationAngle * 180.0 / pi,
              ballRotationAxis.x,
              ballRotationAxis.y,
              ballRotationAxis.z);

    // Draw with red and green stripes only
    for (int i = 0; i < stacks; i++)
    {
        double lat0 = pi * (-0.5 + (double)(i) / stacks);
        double z0 = sin(lat0);
        double zr0 = cos(lat0);

        double lat1 = pi * (-0.5 + (double)(i + 1) / stacks);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++)
        {
            double lng = 2 * pi * (double)(j) / slices;
            double x = cos(lng);
            double y = sin(lng);

            // Only red and green stripes (alternating)
            if ((i % 2) == 0)
            {
                glColor3f(0.9, 0.1, 0.1); // Red
            }
            else
            {
                glColor3f(0.1, 0.9, 0.1); // Green
            }

            glVertex3f(x * zr0 * radius, y * zr0 * radius, z0 * radius);
            glVertex3f(x * zr1 * radius, y * zr1 * radius, z1 * radius);
        }
        glEnd();
    }

    glPopMatrix();
    gluDeleteQuadric(quadric);
}

// Draw an arrow showing velocity direction
void drawVelocityArrow()
{
    if (!showVelocityArrow || magnitude(ballVel) < 0.1)
    {
        return;
    }

    point normVel = ballVel;
    normalize(&normVel);

    double arrowLength = BALL_RADIUS * 2 * ARROW_SCALE * magnitude(ballVel) / initialSpeed;

    // Draw arrow line
    glColor3f(1.0, 1.0, 0.0); // Yellow
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(ballPos.x, ballPos.y, ballPos.z);
    glVertex3f(ballPos.x + normVel.x * arrowLength,
               ballPos.y + normVel.y * arrowLength,
               ballPos.z + normVel.z * arrowLength);
    glEnd();
    glLineWidth(1.0);

    // Draw arrow head
    glPushMatrix();
    glTranslatef(ballPos.x + normVel.x * arrowLength,
                 ballPos.y + normVel.y * arrowLength,
                 ballPos.z + normVel.z * arrowLength);

    // Rotate to point in velocity direction
    point up = {0, 0, 1};
    double angle = acos(normVel.z) * 180.0 / pi;
    point rotAxis = crossProduct(up, normVel);
    if (magnitude(rotAxis) < 0.001)
    {
        rotAxis.x = 1;
        rotAxis.y = 0;
        rotAxis.z = 0;
    }
    normalize(&rotAxis);

    glRotatef(angle, rotAxis.x, rotAxis.y, rotAxis.z);

    // Draw the cone
    GLUquadric *quadric = gluNewQuadric();
    gluCylinder(quadric, BALL_RADIUS * 0.3 * ARROW_SCALE, 0, BALL_RADIUS * ARROW_SCALE, 10, 1);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

void handleBallPhysics(double dt)
{
    if (!simRunning)
    {
        return;
    }

    // Previous position for calculating rolling
    point prevPos = ballPos;

    // Apply gravity
    ballVel.z -= GRAVITY * dt;

    // Update position
    ballPos.x += ballVel.x * dt;
    ballPos.y += ballVel.y * dt;
    ballPos.z += ballVel.z * dt;

    double halfCube = CUBE_SIZE / 2;

    // Check collision with walls and apply bounce
    if (ballPos.x - BALL_RADIUS < -halfCube)
    {
        ballPos.x = -halfCube + BALL_RADIUS;
        ballVel.x = -ballVel.x * RESTITUTION;
    }
    if (ballPos.x + BALL_RADIUS > halfCube)
    {
        ballPos.x = halfCube - BALL_RADIUS;
        ballVel.x = -ballVel.x * RESTITUTION;
    }

    if (ballPos.y - BALL_RADIUS < -halfCube)
    {
        ballPos.y = -halfCube + BALL_RADIUS;
        ballVel.y = -ballVel.y * RESTITUTION;
    }
    if (ballPos.y + BALL_RADIUS > halfCube)
    {
        ballPos.y = halfCube - BALL_RADIUS;
        ballVel.y = -ballVel.y * RESTITUTION;
    }

    // Floor collision
    if (ballPos.z - BALL_RADIUS < 0)
    {
        ballPos.z = BALL_RADIUS;

        // Only bounce if velocity is significant
        if (fabs(ballVel.z) > MIN_VELOCITY)
        {
            ballVel.z = -ballVel.z * RESTITUTION;
        }
        else
        {
            ballVel.z = 0;
        }
    }

    // Ceiling collision
    if (ballPos.z + BALL_RADIUS > halfCube)
    {
        ballPos.z = halfCube - BALL_RADIUS;
        ballVel.z = -ballVel.z * RESTITUTION;
    }

    // Calculate rolling rotation
    if (magnitude(ballVel) > 0.01)
    {
        // Calculate displacement vector
        point displacement;
        displacement.x = ballPos.x - prevPos.x;
        displacement.y = ballPos.y - prevPos.y;
        displacement.z = ballPos.z - prevPos.z;

        // Only calculate rotation if there's actual displacement
        if (magnitude(displacement) > 0.001)
        {
            // Calculate rotation axis (perpendicular to displacement)
            // For a ball rolling on a surface, the rotation axis is perpendicular
            // to both the velocity and the up vector
            point velXY = {ballVel.x, ballVel.y, 0}; // Projected velocity on XY plane
            normalize(&velXY);

            point up = {0, 0, 1};
            ballRotationAxis = crossProduct(velXY, up);
            normalize(&ballRotationAxis);

            // Calculate rotation angle based on arc length
            double displacement2D = sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
            ballRotationAngle += displacement2D / BALL_RADIUS;
        }
    }

    // Damping - slow the ball down over time (simple air resistance)
    double damping = 0.998;
    ballVel.x *= damping;
    ballVel.y *= damping;
    ballVel.z *= damping;

    // Stop the ball if it's moving too slowly
    if (ballPos.z <= BALL_RADIUS + 0.01 && magnitude(ballVel) < MIN_VELOCITY)
    {
        ballVel.x = 0;
        ballVel.y = 0;
        ballVel.z = 0;
    }
}

void drawSS()
{
    // Draw the room (cube)
    drawCube();

    // Draw the floor
    drawFloor();

    // Draw the ball at its current position
    glPushMatrix();
    glTranslatef(ballPos.x, ballPos.y, ballPos.z);
    int number_of_ball_stripes = 30;
    drawBall(BALL_RADIUS, number_of_ball_stripes, number_of_ball_stripes);
    glPopMatrix();

    // Draw velocity arrow
    drawVelocityArrow();
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
        pos.x += l.x * 2;
        pos.y += l.y * 2;
        pos.z += l.z * 2;
        break;

    case 's':
        pos.x -= l.x * 2;
        pos.y -= l.y * 2;
        pos.z -= l.z * 2;
        break;

    case 'd': // Toggle grid
        drawgrid = 1 - drawgrid;
        break;

    case 'a': // Toggle axes
        drawaxes = 1 - drawaxes;
        break;

    // Ball simulation controls
    case ' ': // Toggle simulation
        simRunning = 1 - simRunning;
        break;

    case 'r': // Reset ball position
        resetBall();
        break;

    case '+': // Increase initial speed
        if (!simRunning)
        {
            initialSpeed += 5.0;
            if (initialSpeed > 100.0)
                initialSpeed = 100.0;

            // Update the ball's velocity with new speed (maintaining direction)
            point dir = ballVel;
            normalize(&dir);
            ballVel.x = dir.x * initialSpeed;
            ballVel.y = dir.y * initialSpeed;
            ballVel.z = dir.z * initialSpeed;
        }
        break;

    case '-': // Decrease initial speed
        if (!simRunning)
        {
            initialSpeed -= 5.0;
            if (initialSpeed < 5.0)
                initialSpeed = 5.0;

            // Update the ball's velocity with new speed (maintaining direction)
            point dir = ballVel;
            normalize(&dir);
            ballVel.x = dir.x * initialSpeed;
            ballVel.y = dir.y * initialSpeed;
            ballVel.z = dir.z * initialSpeed;
        }
        break;

    case 'v': // Toggle velocity arrow
        showVelocityArrow = 1 - showVelocityArrow;
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
        pos.x -= r.x * 2;
        pos.y -= r.y * 2;
        pos.z -= r.z * 2;
        break;
    case GLUT_KEY_LEFT:
        pos.x += r.x * 2;
        pos.y += r.y * 2;
        pos.z += r.z * 2;
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
    // Update physics (60fps simulation rate)
    static double lastTime = 0;
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0; // Convert to seconds
    double deltaTime = currentTime - lastTime;

    // Limit physics updates to a reasonable time step
    if (deltaTime > 0.016)
    { // Cap at 60fps (16ms)
        handleBallPhysics(deltaTime);
        lastTime = currentTime;
    }

    glutPostRedisplay();
}

void init()
{
    // Initialize random seed
    srand(time(NULL));

    // Initial values
    drawgrid = 0;
    drawaxes = 0;
    simRunning = 0;
    showVelocityArrow = 1;

    // Reset ball position and velocity
    resetBall();

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

    // Enable lighting for better 3D appearance
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

    glutCreateWindow("3D Bouncing Ball Simulation");

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